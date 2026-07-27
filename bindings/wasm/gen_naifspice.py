#!/usr/bin/env python3
"""
Generate the data the ``naifspice`` JS namespace needs from CSPICE's public
prototype header (``SpiceZpr.h``).

The WASM ``naifspice`` namespace exposes every CSPICE ``*_c`` function as a JS
function (e.g. ``spiceql.naifspice.spkez_c``). Rather than hand-write ~650
wrappers, we parse the prototypes once at build time into a machine-readable
signature table and let a small generic marshaller (bindings/wasm/naifspice.js)
synthesize the functions at load time.

Outputs (written next to the build's other wasm artifacts):
  - ``naifspice_sigs.json``   the signature table consumed by naifspice.js
  - ``naifspice_exports.txt`` one ``_<fn>`` per line, fed to the linker's
                              ``-sEXPORTED_FUNCTIONS=@file`` so the symbols are
                              not dead-stripped (SpiceQL itself only calls a
                              handful of them directly).

Signature-table schema (one object per function, keyed by name)::

  {
    "ret": "<type>",                     # 'void' | 'SpiceDouble' | 'SpiceInt' | 'SpiceBoolean' | 'SpiceChar*'
    "params": [
      {"name": "..", "type": "SpiceDouble", "role": "in|out",
       "ptr": false, "dims": [], "kind": "scalar|string|array|matrix",
       "len_from": <int>}               # (out strings only) index of the capacity arg
    ],
    "ergonomic": true|false              # false -> only the raw-pointer form is usable
  }

The classification here is the single source of truth for whether a function
gets an ergonomic wrapper; naifspice.js just executes what this describes.

Ergonomic contract (what naifspice.js implements from this table):
  - The wrapper takes the INPUT params in declaration order. Scalars are JS
    numbers/booleans; strings are JS strings; arrays/matrices are JS (nested)
    arrays sized by the caller. OUTPUT params are allocated internally and
    returned (single out -> returned directly; several -> an object keyed by
    param name, with the C return value under `return` when non-void).
  - A function is ergonomic only when every parameter is one the marshaller can
    size statically or from the caller's own value. Runtime-sized OUTPUT buffers
    (a bare ``SpiceDouble *values`` sized by a separate ``maxn``/``room`` arg, or
    an unsized ``[][3]`` buffer) cannot be sized safely, so those functions are
    marked non-ergonomic and are reachable only through the raw-pointer form.
"""

import json
import re
import sys

# CSPICE aggregate / callback types a generic marshaller cannot ergonomically
# handle. Functions touching these are still exported and callable in raw
# (numeric pointer) form, but get ergonomic=False so naifspice.js does not try
# to marshal them.
OPAQUE_TYPES = ("SpiceCell", "SpiceEllipse", "SpicePlane", "SpiceDLADescr",
                "SpiceDSKDescr", "SpiceCK", "SpiceEK", "void")

# A single prototype: "<ret> <name>_c ( <args> ) ;"  spanning multiple lines.
PROTO_RE = re.compile(
    r"(?P<ret>(?:Const)?Spice\w+\s*\*?|void)\s+"
    r"(?P<name>[a-z][a-z0-9_]*_c)\s*"
    r"\((?P<args>[^;]*?)\)\s*;",
    re.DOTALL,
)


def _norm(s):
    return re.sub(r"\s+", " ", s).strip()


def parse_return(ret):
    ret = _norm(ret)
    if ret.endswith("*"):
        # Only tkvrsn_c: ConstSpiceChar * (a returned C string).
        return "SpiceChar*"
    return ret


def parse_param(raw):
    """Parse one parameter declaration into a descriptor dict."""
    raw = _norm(raw)
    is_const = "Const" in raw
    is_ptr = "*" in raw
    # Trailing [..] dimensions, e.g. state[6] or rotate[3][3].
    dims = [int(d) for d in re.findall(r"\[(\d+)\]", raw)]
    has_empty_dim = "[]" in raw

    # Base type = the Spice* / void token.
    tmatch = re.search(r"(Const)?(Spice\w+|void)", raw)
    base = tmatch.group(2) if tmatch else "void"

    # Parameter name = last identifier before any '[' (f2c protos always name args).
    stripped = re.sub(r"\[.*$", "", raw).replace("*", " ")
    idents = re.findall(r"[A-Za-z_]\w*", stripped)
    # Drop leading type keywords to isolate the name.
    name = idents[-1] if idents and idents[-1] not in ("Const", base) else ""

    # Role: Const => input; a bare value => input; non-const pointer/array => output.
    if is_const:
        role = "in"
    elif is_ptr or dims:
        role = "out"
    else:
        role = "in"

    kind = "scalar"
    opaque = base in OPAQUE_TYPES or "SpiceCell" in raw
    if "(*" in raw or ") (" in raw or ")(" in raw:
        kind = "fnptr"
    elif opaque:
        kind = "opaque"
    elif base == "SpiceChar":
        kind = "string"
    elif len(dims) >= 2:
        kind = "matrix"
    elif dims:
        kind = "array"           # fixed-length numeric array, e.g. state[6]
    elif is_ptr:
        # Bare numeric pointer, no [] dims. As an INPUT it is a variable-length
        # array the caller sizes (e.g. ConstSpiceDouble *v1); as an OUTPUT it is
        # a single scalar passed by reference (e.g. SpiceDouble *et). A bare
        # OUTPUT buffer sized by a companion capacity arg is caught in
        # is_ergonomic() and pushed to the raw path.
        kind = "array" if role == "in" else "scalar"

    return {
        "name": name,
        "type": base,
        "const": is_const,
        "ptr": is_ptr,
        "dims": dims,
        "empty_dim": has_empty_dim,
        "kind": kind,
        "role": role,
    }


# Substrings marking a SpiceInt input that carries a *count/capacity* rather
# than a data value. When an output buffer's size depends on one of these at
# runtime, the marshaller cannot size the allocation statically, so the function
# is pushed to the raw path (guarding against a heap overflow from an
# undersized allocation, e.g. bodvrd_c writing maxn doubles into 8 bytes).
CAPACITY_HINTS = ("room", "maxn", "max", "ndim", "nmax", "size", "nrow", "ncol")


def _has_capacity_input(params):
    for p in params:
        if (p["type"] == "SpiceInt" and p["role"] == "in" and not p["ptr"]
                and any(h in p["name"].lower() for h in CAPACITY_HINTS)):
            return True
    return False


def is_ergonomic(params):
    """A function is ergonomic if every parameter is one the marshaller handles
    with a plain JS value: input scalars/strings/(nested) arrays, fixed-size
    array/matrix outputs, output scalars, and length-known output strings.
    Everything else (opaque/callback types, runtime-sized output buffers) is
    reachable only through the raw-pointer form."""
    has_capacity = _has_capacity_input(params)
    for i, p in enumerate(params):
        if p["kind"] in ("opaque", "fnptr"):
            return False
        if p["kind"] == "string" and p["role"] == "out":
            # An output string needs a capacity: an int input named like *len*.
            if _find_len_arg(params, i) is None:
                return False
        if p["kind"] == "array" and p["role"] == "out" and not p["dims"]:
            # Bare output buffer (e.g. bodvrd values[], gdpool values): its
            # length is a runtime capacity arg — cannot size safely.
            return False
        if p["kind"] == "matrix" and p["role"] == "out" and 0 in p["dims"]:
            # Unsized leading dimension, e.g. getfov bounds[][3].
            return False
        if p["kind"] == "scalar" and p["role"] == "out" and p["ptr"] \
                and p["type"] in ("SpiceDouble", "SpiceInt") and has_capacity:
            # A bare numeric out-pointer alongside a capacity arg is almost
            # always a buffer the parser under-sized to a single scalar (e.g.
            # gdpool_c values, bodvrd_c values). Be conservative: raw path.
            return False
    return True


def _find_len_arg(params, str_index):
    """Index of the SpiceInt input that gives an output string's capacity.
    CSPICE names these lenout/namelen/lenvals/... — match an int input whose
    name contains 'len'."""
    for j, p in enumerate(params):
        if (p["type"] == "SpiceInt" and p["role"] == "in"
                and not p["ptr"] and "len" in p["name"].lower()):
            return j
    return None


def defined_symbols(src_dir):
    """Set of *_c functions that are actually *defined* in the CSPICE source
    tree (not merely prototyped in the header). CSPICE ships one function per
    <name>.c file, but a few internal helpers (e.g. zzgfgeth_c) are defined
    inside a differently-named file, and a few header prototypes have no
    definition at all in this package (e.g. prefix_c). We only export symbols
    that exist, or the linker (ERROR_ON_UNDEFINED_SYMBOLS) fails."""
    import glob
    import os
    defined = set()
    # f2c definitions are indented and put the return type on the name's line,
    # e.g. "   SpiceBoolean zzgfgeth_c ( void )". A call has no return-type
    # token immediately before the name, so this won't match call sites.
    def_re = re.compile(
        r"^\s*(?:Const)?Spice\w+\s*\*?\s+([a-z][a-z0-9_]*_c)\s*\(|"
        r"^\s*void\s+([a-z][a-z0-9_]*_c)\s*\(",
        re.MULTILINE)
    for path in glob.glob(os.path.join(src_dir, "*.c")):
        base = os.path.splitext(os.path.basename(path))[0]
        if base.endswith("_c"):
            defined.add(base)  # one-function-per-file convention
        # Also scan for definitions that live in another file.
        try:
            with open(path, "r", errors="replace") as fh:
                for m in def_re.finditer(fh.read()):
                    defined.add(m.group(1) or m.group(2))
        except OSError:
            pass
    return defined


def main(header_path, sigs_out, exports_out, src_dir=None):
    with open(header_path, "r", errors="replace") as fh:
        text = fh.read()

    sigs = {}
    for m in PROTO_RE.finditer(text):
        name = m.group("name")
        args = m.group("args").strip()
        if args in ("", "void"):
            params = []
        else:
            params = [parse_param(a) for a in args.split(",")]
        # Attach the capacity-source index to output strings for the marshaller.
        for i, p in enumerate(params):
            if p["kind"] == "string" and p["role"] == "out":
                li = _find_len_arg(params, i)
                if li is not None:
                    p["len_from"] = li
        sigs[name] = {
            "ret": parse_return(m.group("ret")),
            "params": params,
            "ergonomic": is_ergonomic(params),
        }

    # The sig table lists every prototype (naifspice.js throws a clear error if a
    # header-only prototype is ever called). The export list, which the linker
    # consumes, must contain only symbols with a real definition.
    if src_dir:
        defined = defined_symbols(src_dir)
        exported = [n for n in sorted(sigs) if n in defined]
        dropped = [n for n in sorted(sigs) if n not in defined]
        if dropped:
            print("naifspice: %d prototype(s) with no definition in this CSPICE "
                  "package, not exported: %s" % (len(dropped), ", ".join(dropped)),
                  file=sys.stderr)
    else:
        exported = sorted(sigs)

    with open(sigs_out, "w") as fh:
        json.dump(sigs, fh, separators=(",", ":"), sort_keys=True)

    with open(exports_out, "w") as fh:
        # _malloc/_free back the raw-pointer path; the rest are the CSPICE funcs.
        fh.write("_malloc\n_free\n")
        for name in exported:
            fh.write("_%s\n" % name)

    ergo = sum(1 for s in sigs.values() if s["ergonomic"])
    print("naifspice: parsed %d CSPICE functions (%d ergonomic, %d raw-only), "
          "%d exported" % (len(sigs), ergo, len(sigs) - ergo, len(exported)),
          file=sys.stderr)
    return 0


if __name__ == "__main__":
    if len(sys.argv) not in (4, 5):
        print("usage: gen_naifspice.py <SpiceZpr.h> <sigs.json> <exports.txt> "
              "[cspice_src_dir]", file=sys.stderr)
        sys.exit(2)
    src = sys.argv[4] if len(sys.argv) == 5 else None
    sys.exit(main(sys.argv[1], sys.argv[2], sys.argv[3], src))
