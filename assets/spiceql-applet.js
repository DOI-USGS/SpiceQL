/**
 * The runnable SpiceQL applet on the home page.
 *
 * Three panes, top to bottom: a stack of kernel cards, the code fence MkDocs
 * highlighted, and the plot. Run furnishes the kernels in the order the cards
 * are in, identifies each one, and only then executes the code.
 *
 * The division of labour is the point: the applet owns the environment — loading
 * the WASM module, mounting kernels, furnishing them in order — and the code
 * fence owns only what a reader would actually write, the exploitation. That is
 * why `spiceql` and `plot` are not imported by the snippet: they are set on
 * globalThis before it runs. Plotly IS imported there, because which plotting
 * library you reach for is a choice the reader makes, not part of the harness.
 *
 * There is still only one copy of the code: the fence in index.md is the source,
 * and it is read back out of the DOM at run time, so the example cannot drift
 * from what executes. It is contenteditable, and every run makes a fresh blob
 * URL, so edits take effect.
 *
 * Nothing here downloads until Run is pressed for the first time.
 */

const DIST = 'https://cdn.jsdelivr.net/npm/@usgs-astrogeology/spiceql/dist/';
const DATA = 'https://asc-isisdata.s3.us-west-2.amazonaws.com/usgs_data';

/**
 * Preloaded as if they had been dropped in. One kernel is genuinely all the
 * example needs: SpiceQL converts UTC to ET with its own leap-second table
 * (SpiceQL/src/utcet.h), not with a furnished LSK, so no naif0012.tls is
 * required here — verified against a run with one furnished, which gives the
 * same ET to the millisecond.
 */
const DEFAULT_KERNELS = [
  {
    name: '120829R_SCPSE_12151_12199.bsp',
    url: `${DATA}/cassini/kernels/spk/120829R_SCPSE_12151_12199.bsp`,
  },
];

/**
 * The api.h calls that take a trailing options object, and so can have
 * `searchKernels: false` supplied for them — see furnishedApi(). Everything else
 * on the module (KernelSet, load, mountKernel, Module, …) is passed through
 * untouched, because its last argument is not an options bag.
 */
const POOL_READING_CALLS = [
  'getTargetStates', 'getTargetStatesRanged',
  'getTargetOrientations', 'getTargetOrientationsRanged',
  'getExactTargetOrientations', 'extractExactCkTimes',
  'strSclkToEt', 'doubleSclkToEt', 'doubleEtToSclk',
  'utcToEt', 'etToUtc',
  'translateNameToCode', 'translateCodeToName',
  'getFrameInfo', 'getTargetFrameInfo',
  'findMissionKeywords', 'findTargetKeywords', 'frameTrace',
];

/**
 * SpiceQL's own extension-to-type table, from formatKernels() in
 * SpiceQL/src/utils.cpp. Only a fallback: getfat() below reports the type from
 * the file's own header for everything it recognises, and this catches the text
 * kernels it answers 'TEXT' for.
 */
const EXT_TYPES = {
  '.bc': 'CK', '.bsp': 'SPK', '.tf': 'FK', '.ti': 'IK', '.tls': 'LSK',
  '.tm': 'MK', '.tpc': 'PCK', '.bpc': 'PCK', '.tsc': 'SCLK', '.iak': 'IAK',
  '.bds': 'DSK', '.bes': 'EK',
};

/** Quality is a SPK/CK notion in SpiceQL; everything else reports N/A. */
const QUALITY_TYPES = ['SPK', 'CK'];

/** The kernel-type level in a config tree, so frame keys can be told from it. */
const CONFIG_TYPES = ['ck', 'spk', 'tspk', 'fk', 'ik', 'iak', 'lsk', 'sclk', 'pck',
                      'dsk', 'ek', 'mk'];

let nextId = 1;

/* ── The WASM module, loaded once, on the first Run ───────────────────────── */

let modulePromise = null;

function loadModule() {
  if (!modulePromise) {
    modulePromise = (async () => {
      const [{ loadSpiceQL }, { loadNaifspice }] = await Promise.all([
        import(`${DIST}spiceql.js`),
        import(`${DIST}naifspice.js`),
      ]);
      const spiceql = await loadSpiceQL({
        moduleOverrides: { locateFile: (f) => DIST + f },
      });
      // The raw CSPICE layer, for getfat below. loadSpiceQL deliberately does not
      // build it, so it is asked for explicitly.
      const naif = loadNaifspice(spiceql);
      return { spiceql, naif, matchers: configMatchers(spiceql) };
    })();
  }
  return modulePromise;
}

/**
 * SpiceQL's own kernel patterns, read out of the config DB that ships inside
 * spiceql_wasm.data — the same metadata the native build searches. What is
 * missing in the WASM build is the HDF5 index over a kernel tree, not these.
 *
 * The tree is {frame: {type: {quality: {kernels: [regex, …]}}}}, with the quality
 * level absent for types that have no qualities (fk, ik, sclk, …). Hence the two
 * labels a card carries besides its type:
 *
 *   mission — the config the pattern came from: cassini, mro, base.
 *   frame   — its top-level key, which SpiceQL keys by frame rather than by kind,
 *             so it is a spacecraft (cassini), an instrument (ctx, hirise) or a
 *             body (mars, titan) depending on what the kernel is for. Often the
 *             same word as the mission, and usefully different when it is not.
 *
 * The patterns are anchored basename regexes and parse as JavaScript RegExp as
 * they stand. `kernels` holds either a list of them or a single bare string —
 * both shapes are in the shipped configs, 254 lists and 78 strings — so a walker
 * that only recursed into arrays silently skipped a quarter of them, which is
 * what left cas00172.tsc reporting no mission.
 */
function configMatchers(spiceql) {
  const dir = '/spiceql/SpiceQL/db';
  const decoder = new TextDecoder();
  const matchers = [];

  let files = [];
  try {
    files = spiceql.Module.FS.readdir(dir).filter((f) => f.endsWith('.json'));
  } catch (err) {
    console.warn('SpiceQL applet: no config DB in the virtual FS', err);
    return matchers;
  }

  for (const file of files) {
    let config;
    try {
      config = JSON.parse(decoder.decode(spiceql.Module.FS.readFile(`${dir}/${file}`)));
    } catch (err) {
      continue;   // a config we cannot read is one we cannot match against
    }
    const configName = file.replace(/\.json$/, '');
    walk(config, []);

    function walk(node, path) {
      const isPatterns = path.at(-1) === 'kernels'
        && (Array.isArray(node) || typeof node === 'string');
      if (isPatterns) {
        const patterns = Array.isArray(node) ? node : [node];
        const [topKey] = path;
        const type = path.find((k) => CONFIG_TYPES.includes(k.toLowerCase()));
        if (!type) return;
        // key/type/quality/kernels carries one; key/type/kernels does not. SpiceQL
        // also writes 'noquality' where a type has the level but no grading.
        const level = path.at(-2);
        const quality = (level === type || level === 'noquality') ? null : level;
        for (const pattern of patterns) {
          try {
            matchers.push({
              type: type.toUpperCase(),
              quality,
              mission: configName,
              frame: topKey,
              re: new RegExp(pattern),
            });
          } catch (err) {
            /* a pattern JS cannot compile simply never matches */
          }
        }
        return;
      }
      if (node && typeof node === 'object') {
        for (const key of Object.keys(node)) walk(node[key], [...path, key]);
      }
    }
  }
  return matchers;
}

/**
 * Type and quality for one mounted kernel.
 *
 * getfat() reads the file's own header — architecture plus type, e.g. DAF/SPK or
 * KPL/LSK — so it works on a kernel that has not been furnished, which is what
 * makes it usable for labelling a card. The two buffer lengths are the C
 * signature's: getfat_c(file, arclen, typlen).
 */
function identify({ naif, matchers }, kernel) {
  let type = null;
  try {
    const fat = naif.getfat(kernel.path, 32, 32);
    type = (fat.type || '').toUpperCase();
  } catch (err) {
    kernel.note = err.message;
  }

  if (!type || type === 'TEXT' || type === '?') {
    const dot = kernel.name.lastIndexOf('.');
    type = (dot > 0 && EXT_TYPES[kernel.name.slice(dot).toLowerCase()]) || type || '?';
  }

  kernel.type = type;

  // One pass over the patterns gives all three labels: the entry that matched
  // knows which config and which key it came from. Type-agnostic on purpose —
  // a kernel whose name matches a pattern filed under another type still tells
  // us its mission, and the type badge is getfat's answer either way.
  const hit = matchers.find((m) => m.re.test(kernel.name))
           || matchers.find((m) => m.type === type && m.re.test(kernel.name));
  kernel.mission = hit?.mission || 'N/A';
  kernel.frame = hit?.frame || 'N/A';
  // Quality is a SPK/CK grading in SpiceQL; anything else reports N/A even when
  // the config happens to file it under one.
  kernel.quality = (QUALITY_TYPES.includes(type) && hit?.quality) || 'N/A';
}

/**
 * The module as the code fence sees it: every pool-reading call defaults to
 * searchKernels:false, because the applet has already furnished the kernels and
 * kernel search throws in the WASM build anyway. A caller's own options still
 * win, so a snippet can pass searchKernels:true to watch it fail, or add a
 * mission, without the wrapper getting in the way.
 */
function furnishedApi(spiceql) {
  const api = Object.create(spiceql);
  for (const name of POOL_READING_CALLS) {
    const fn = spiceql[name];
    if (typeof fn !== 'function') continue;
    api[name] = (...args) => {
      const last = args.at(-1);
      const isOpts = last !== null && typeof last === 'object' && !Array.isArray(last);
      const opts = { searchKernels: false, ...(isOpts ? last : {}) };
      return fn(...(isOpts ? args.slice(0, -1) : args), opts);
    };
  }
  return api;
}

/* ── One applet ───────────────────────────────────────────────────────────── */

function setUp(root) {
  const stack = root.querySelector('[data-sq-stack]');
  const button = root.querySelector('[data-sq-run]');
  const status = root.querySelector('[data-sq-status]');
  const restore = root.querySelector('[data-sq-restore]');
  const plot = root.querySelector('[data-sq-plot]');
  const code = root.querySelector('.highlight code');
  if (!stack || !button || !code) return;

  code.contentEditable = 'true';
  code.spellcheck = false;
  code.setAttribute('aria-label', 'Editable SpiceQL example, run with the Run button');

  /** @type {{id:number,name:string,url?:string,file?:File,bytes?:Uint8Array,size?:number,path?:string,type?:string,quality?:string}[]} */
  let kernels = [];
  let objectUrl = null;
  let dragId = null;

  const say = (text, isError = false) => {
    status.textContent = text;
    root.classList.toggle('has-error', isError);
  };

  const fmtSize = (bytes) =>
    bytes === undefined ? ''
      : bytes >= 1048576 ? `${(bytes / 1048576).toFixed(1)} MB`
      : `${Math.max(1, Math.round(bytes / 1024))} KB`;

  function render() {
    stack.textContent = '';
    if (!kernels.length) {
      const li = document.createElement('li');
      li.className = 'sq-app__empty';
      li.textContent = 'No kernels. Drop .bsp, .bc, .tls, .tpc, .tf, .ti or .tsc files here.';
      stack.append(li);
      return;
    }

    kernels.forEach((kernel, index) => {
      const li = document.createElement('li');
      li.className = 'sq-kcard';
      li.draggable = true;
      li.tabIndex = 0;
      li.dataset.id = String(kernel.id);
      li.title = `${kernel.name}\n${kernel.url || 'dropped file'}`;
      li.setAttribute('aria-label',
        `${index + 1}: ${kernel.name}, ${kernel.type
          ? `${kernel.type}, quality ${kernel.quality}, mission ${kernel.mission},`
            + ` frame ${kernel.frame}`
          : 'not yet identified'}.`
        + ' Arrow keys reorder, Delete removes.');

      const n = document.createElement('span');
      n.className = 'sq-kcard__n';
      n.textContent = String(index + 1);

      const name = document.createElement('span');
      name.className = 'sq-kcard__name';
      name.textContent = kernel.name;

      // Second row: the identification. Every fact carries its own label,
      // because 'cassini' and 'ctx' beside each other say nothing about which is
      // the mission and which the frame. Before the first run there are no bytes
      // to look at, so each is an em dash — honestly unknown rather than guessed
      // from the file name.
      const meta = document.createElement('span');
      meta.className = 'sq-kcard__meta';
      const facts = [
        ['type', kernel.type, true],
        ['quality', kernel.quality],
        ['mission', kernel.mission],
        ['frame', kernel.frame],
      ];
      for (const [label, value, isType] of facts) {
        const fact = document.createElement('span');
        fact.className = 'sq-kfact';
        if (isType && kernel.type) fact.classList.add('is-type');
        const key = document.createElement('i');
        key.textContent = label;
        fact.append(key, document.createTextNode(value || '—'));
        meta.append(fact);
      }
      if (kernel.size !== undefined) {
        const size = document.createElement('span');
        size.className = 'sq-kfact';
        size.append(document.createTextNode(fmtSize(kernel.size)));
        meta.append(size);
      }

      const remove = document.createElement('button');
      remove.className = 'sq-kcard__x';
      remove.type = 'button';
      remove.innerHTML = '&times;';
      remove.setAttribute('aria-label', `Remove ${kernel.name}`);
      remove.addEventListener('click', (event) => {
        event.stopPropagation();
        kernels = kernels.filter((k) => k.id !== kernel.id);
        render();
      });

      // Row 1 is the handle, the name and the remove button; row 2 is the meta.
      const head = document.createElement('span');
      head.className = 'sq-kcard__head';
      head.append(n, name, remove);
      li.append(head, meta);
      stack.append(li);
    });
  }

  /* Reordering. Two drag sources land on this stack — a card being moved and
     files from the desktop — so they are told apart by what the drag carries:
     our own MIME type for a card, 'Files' for a drop from outside. */

  stack.addEventListener('dragstart', (event) => {
    const card = event.target.closest('.sq-kcard');
    if (!card) return;
    dragId = Number(card.dataset.id);
    event.dataTransfer.setData('application/x-sq-kernel', card.dataset.id);
    event.dataTransfer.effectAllowed = 'move';
    card.classList.add('is-dragging');
  });

  stack.addEventListener('dragend', () => {
    dragId = null;
    stack.querySelectorAll('.sq-kcard').forEach((c) =>
      c.classList.remove('is-dragging', 'is-over'));
  });

  stack.addEventListener('dragover', (event) => {
    if (dragId === null) return;
    event.preventDefault();
    event.dataTransfer.dropEffect = 'move';
    const card = event.target.closest('.sq-kcard');
    stack.querySelectorAll('.sq-kcard').forEach((c) => c.classList.remove('is-over'));
    if (card && Number(card.dataset.id) !== dragId) card.classList.add('is-over');
  });

  stack.addEventListener('drop', (event) => {
    if (dragId === null) return;
    event.preventDefault();
    event.stopPropagation();               // not a file drop; keep it off the root
    const card = event.target.closest('.sq-kcard');
    const from = kernels.findIndex((k) => k.id === dragId);
    let to = card ? kernels.findIndex((k) => k.id === Number(card.dataset.id)) : kernels.length - 1;
    if (from < 0 || to < 0 || from === to) { dragId = null; render(); return; }
    const [moved] = kernels.splice(from, 1);
    kernels.splice(to, 0, moved);
    dragId = null;
    render();
    say(`Moved ${moved.name} to position ${to + 1}. Run to furnish in the new order.`);
  });

  // The same moves without a mouse.
  stack.addEventListener('keydown', (event) => {
    const card = event.target.closest('.sq-kcard');
    if (!card) return;
    const id = Number(card.dataset.id);
    const at = kernels.findIndex((k) => k.id === id);
    if (at < 0) return;

    if (event.key === 'ArrowLeft' || event.key === 'ArrowRight') {
      const to = at + (event.key === 'ArrowLeft' ? -1 : 1);
      if (to < 0 || to >= kernels.length) return;
      event.preventDefault();
      [kernels[at], kernels[to]] = [kernels[to], kernels[at]];
      render();
      stack.querySelector(`[data-id="${id}"]`)?.focus();
    } else if (event.key === 'Delete' || event.key === 'Backspace') {
      event.preventDefault();
      kernels = kernels.filter((k) => k.id !== id);
      render();
      stack.querySelector('.sq-kcard')?.focus();
    }
  });

  /* File drops, anywhere on the applet. */

  root.addEventListener('dragover', (event) => {
    if (dragId !== null || !event.dataTransfer.types.includes('Files')) return;
    event.preventDefault();
    root.classList.add('is-dropping');
  });

  root.addEventListener('dragleave', (event) => {
    if (event.target === root) root.classList.remove('is-dropping');
  });

  root.addEventListener('drop', (event) => {
    if (dragId !== null || !event.dataTransfer.types.includes('Files')) return;
    event.preventDefault();
    root.classList.remove('is-dropping');
    const files = [...event.dataTransfer.files];
    if (!files.length) return;
    let replaced = 0;
    for (const file of files) {
      const card = { id: nextId++, name: file.name, file, size: file.size };
      // A second card with the same name would mount over the first — same
      // virtual-FS path — and then furnish it twice, so the newer bytes replace
      // the older card in place and keep its position in the order.
      const at = kernels.findIndex((k) => k.name === file.name);
      if (at >= 0) { kernels[at] = card; replaced += 1; } else kernels.push(card);
    }
    render();
    const what = files.length === 1 ? files[0].name : `${files.length} kernels`;
    say(`${replaced === files.length ? 'Replaced' : 'Added'} ${what}.`
        + ' Run to furnish and identify.');
  });

  restore?.addEventListener('click', () => {
    kernels = DEFAULT_KERNELS.map((k) => ({ ...k, id: nextId++ }));
    render();
    say('Kernel stack reset to the example’s own two kernels.');
  });

  /* Run: fetch, mount, furnish in order, identify, then execute the fence. */

  button.addEventListener('click', async () => {
    button.disabled = true;
    root.classList.remove('has-error');
    const started = performance.now();

    try {
      say('Loading the SpiceQL module…');
      const env = await loadModule();
      const { spiceql } = env;

      for (const kernel of kernels) {
        if (kernel.bytes) continue;
        say(`Fetching ${kernel.name}…`);
        if (kernel.file) {
          kernel.bytes = new Uint8Array(await kernel.file.arrayBuffer());
        } else {
          const response = await fetch(kernel.url);
          if (!response.ok) throw new Error(`${kernel.name}: HTTP ${response.status}`);
          kernel.bytes = new Uint8Array(await response.arrayBuffer());
        }
        kernel.size = kernel.bytes.byteLength;
      }

      // Unfurnish whatever the last run left in the pool, newest first, so a
      // reordered or shortened stack is not layered on top of the old one.
      for (const path of [...spiceql.getLoadedKernels()].reverse()) {
        try { spiceql.unload(path); } catch (err) { /* already gone */ }
      }

      say('Furnishing kernels…');
      for (const kernel of kernels) {
        kernel.path = spiceql.mountKernel(`/kernels/${kernel.name}`, kernel.bytes);
        spiceql.load(kernel.path);          // one at a time: the order is the point
        identify(env, kernel);
      }
      render();

      if (objectUrl) URL.revokeObjectURL(objectUrl);
      objectUrl = URL.createObjectURL(new Blob([code.innerText], { type: 'text/javascript' }));

      // What the fence gets: the furnished module and the plot element. Set on
      // globalThis because a blob module has no other channel — imports are the
      // only other way in, and the point is that the snippet does not import
      // the harness.
      globalThis.spiceql = furnishedApi(spiceql);
      globalThis.plot = plot;
      globalThis.kernels = kernels.map(({ name, type, quality, mission, frame }) =>
        ({ name, type, quality, mission, frame }));

      say('Running…');
      await import(objectUrl);

      const secs = ((performance.now() - started) / 1000).toFixed(1);
      const furnished = kernels.length === 1 ? '1 kernel' : `${kernels.length} kernels`;
      say(`Done in ${secs}s with ${furnished} furnished. Drag the plot to rotate it.`);
    } catch (err) {
      // Everything lands here: a failed fetch, a SPICE error out of the wasm, a
      // syntax error in the edited fence, or anything its top-level await threw.
      console.error(err);
      say(`${err.name || 'Error'}: ${err.message}`, true);
    } finally {
      button.disabled = false;
    }
  });

  kernels = DEFAULT_KERNELS.map((k) => ({ ...k, id: nextId++ }));
  render();
}

document.querySelectorAll('[data-sq-app]').forEach(setUp);
