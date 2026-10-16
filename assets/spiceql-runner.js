/**
 * Makes a code fence runnable against the SpiceQL WASM build.
 *
 * The division of labour is the same one the REST applet uses, for the same
 * reason: the harness owns the environment and the fence owns only what a reader
 * would actually write. Here the environment is the bigger half — loading a
 * 2.4 MB wasm module, fetching kernels, writing them into the virtual FS and
 * furnishing them — and none of it belongs in an example about calling api.h.
 * So the fence gets `spiceql`, already furnished, on globalThis.
 *
 * What replaces that code is the pane above it. Instead of a request log it
 * shows one card per file the run needs, with a progress bar, because the
 * honest answer to "what does this cost" is three megabytes and a reader should
 * watch it arrive rather than find out from the network tab.
 *
 * Nothing downloads until Run is pressed. Everything is fetched with
 * cache:'force-cache', so a second run — or a second visit — is instant and the
 * cards say so rather than re-reporting a download that did not happen.
 *
 * The module's own .wasm and .data are deliberately fetched here, before
 * loadSpiceQL() is called, even though loadSpiceQL would fetch them itself: the
 * Emscripten loader gives no progress, and fetching them first both fills the
 * HTTP cache it then reads from and lets those two files appear as cards like
 * everything else.
 */

const DIST = 'https://cdn.jsdelivr.net/npm/@usgs-astrogeology/spiceql/dist/';
const DATA = 'https://asc-isisdata.s3.us-west-2.amazonaws.com/usgs_data';

/**
 * Prefetched for progress, then left to the Emscripten loader. Only the two big
 * binaries: spiceql_wasm.js is 200 KB of glue that the module import pulls in
 * anyway, and a card for it would be noise.
 */
const MODULE_ASSETS = [
  { name: 'spiceql_wasm.wasm', url: `${DIST}spiceql_wasm.wasm`, note: 'WASM module' },
  { name: 'spiceql_wasm.data', url: `${DIST}spiceql_wasm.data`, note: 'config DB' },
];

/**
 * The kernels the example needs, furnished in this order.
 *
 * One 115 KB text kernel, which is why the example is the CTX frame query and
 * not a trajectory: the rotation between CTX and the MRO bus is a fixed mounting
 * offset written down in the frame kernel, so mro_v16.tf answers it on its own.
 * Verified against the hosted service, which returns the same quaternion to
 * every digit out of a ten-kernel set including a CK and de430 — the difference
 * is only what each end had to load to get there.
 *
 * The service's own search picks mro_v17.tf; v16 is the newest in the public
 * bucket a browser can fetch, and the frame chain this uses is the same in both.
 */
const KERNELS = [
  {
    name: 'mro_v16.tf',
    url: `${DATA}/mro/kernels/fk/mro_v16.tf`,
    note: 'MRO frame definitions',
  },
];

/**
 * The api.h calls that take a trailing options object, and so can have
 * `searchKernels: false` supplied for them. Everything else on the module
 * (KernelSet, mountKernel, Module, …) is passed through untouched, because its
 * last argument is not an options bag.
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

const fmtSize = (bytes) =>
  bytes >= 1048576 ? `${(bytes / 1048576).toFixed(1)} MB`
    : `${Math.max(1, Math.round(bytes / 1024))} KB`;

/**
 * Fetches with byte-level progress. The stream is read rather than awaited whole
 * because content-length alone cannot say how far along a 2.4 MB download is,
 * and that is the number the card exists to show.
 */
async function fetchWithProgress(url, onProgress) {
  const response = await fetch(url, { cache: 'force-cache' });
  if (!response.ok) throw new Error(`HTTP ${response.status}`);
  const total = Number(response.headers.get('content-length')) || 0;

  if (!response.body) {                     // no streams: one jump to done
    const bytes = new Uint8Array(await response.arrayBuffer());
    onProgress(bytes.length, bytes.length);
    return bytes;
  }

  const reader = response.body.getReader();
  const chunks = [];
  let received = 0;
  for (;;) {
    const { done, value } = await reader.read();
    if (done) break;
    chunks.push(value);
    received += value.length;
    onProgress(received, total);
  }

  const bytes = new Uint8Array(received);
  let at = 0;
  for (const chunk of chunks) { bytes.set(chunk, at); at += chunk.length; }
  return bytes;
}

/**
 * Whether the browser served that URL out of cache rather than the network.
 * Resource Timing is the only thing that knows — a cached response is a normal
 * 200 with a normal body, and reports transferSize 0 with a non-zero decoded
 * size. Wrapped because the entry is absent if the buffer has been cleared.
 */
function cameFromCache(url) {
  try {
    const entry = performance.getEntriesByName(url).at(-1);
    return !!entry && entry.transferSize === 0 && entry.decodedBodySize > 0;
  } catch (err) {
    return false;
  }
}

/* ── The module, loaded once, on the first Run ────────────────────────────── */

let modulePromise = null;

function loadModule() {
  if (!modulePromise) {
    modulePromise = (async () => {
      const { loadSpiceQL } = await import(`${DIST}spiceql.js`);
      return loadSpiceQL({ moduleOverrides: { locateFile: (f) => DIST + f } });
    })();
  }
  return modulePromise;
}

/**
 * The module as the fence sees it: every pool-reading call defaults to
 * searchKernels:false, because the harness has already furnished the kernels and
 * kernel search throws in the WASM build anyway. A caller's own options still
 * win, so a snippet can pass searchKernels:true to watch it fail without the
 * wrapper getting in the way.
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

/* ── One runnable fence ───────────────────────────────────────────────────── */

function setUp(root) {
  const button = root.querySelector('[data-sq-run]');
  const status = root.querySelector('[data-sq-status]');
  const stack = root.querySelector('[data-sq-downloads]');
  const out = root.querySelector('[data-sq-out]');
  const code = root.querySelector('.highlight code');
  if (!button || !out || !code) return;

  code.contentEditable = 'true';
  code.spellcheck = false;
  code.setAttribute('aria-label', 'Editable SpiceQL example, run with the Run button');

  let objectUrl = null;
  let furnished = null;          // the live KernelSet, so a re-run can replace it

  const say = (text, isError = false) => {
    if (status) status.textContent = text;
    root.classList.toggle('has-error', isError);
  };

  /* The download cards. Each file gets one up front, in the state 'waiting', so
     the pane shows what a run is going to cost before it starts costing it. */

  function card(file) {
    const li = document.createElement('li');
    li.className = 'sq-kcard';
    li.title = file.url;

    const head = document.createElement('span');
    head.className = 'sq-kcard__head';
    const name = document.createElement('span');
    name.className = 'sq-kcard__name';
    name.textContent = file.name;
    head.append(name);

    const meta = document.createElement('span');
    meta.className = 'sq-kcard__meta';
    const note = document.createElement('span');
    note.className = 'sq-kfact';
    note.textContent = file.note;
    const state = document.createElement('span');
    state.className = 'sq-kfact';
    state.textContent = 'waiting';
    meta.append(note, state);

    const track = document.createElement('span');
    track.className = 'sq-kbar';
    const fill = document.createElement('span');
    fill.className = 'sq-kbar__fill';
    track.append(fill);

    li.append(head, meta, track);
    return { li, state, fill, track };
  }

  function renderCards(files) {
    stack.textContent = '';
    const cards = new Map();
    for (const file of files) {
      const made = card(file);
      cards.set(file.name, made);
      stack.append(made.li);
    }
    return cards;
  }

  /** Downloads every file, in order, updating its card as the bytes arrive. */
  async function download(files, cards) {
    const bytes = new Map();
    for (const file of files) {
      const ui = cards.get(file.name);
      ui.state.textContent = 'fetching';
      ui.li.classList.add('is-active');
      try {
        const data = await fetchWithProgress(file.url, (received, total) => {
          const frac = total ? received / total : 0;
          ui.fill.style.width = `${Math.round(frac * 100)}%`;
          ui.state.textContent = total
            ? `${fmtSize(received)} of ${fmtSize(total)}`
            : fmtSize(received);
        });
        bytes.set(file.name, data);
        ui.fill.style.width = '100%';
        ui.li.classList.remove('is-active');
        ui.li.classList.add('is-done');
        // Reported after the fact, because a cache hit is only distinguishable
        // from a fast network once the response has been accounted for.
        ui.state.textContent = cameFromCache(file.url)
          ? `${fmtSize(data.length)} from cache`
          : fmtSize(data.length);
        if (cameFromCache(file.url)) ui.state.classList.add('is-type');
      } catch (err) {
        ui.li.classList.remove('is-active');
        ui.li.classList.add('is-failed');
        ui.state.textContent = err.message;
        throw new Error(`${file.name}: ${err.message}`);
      }
    }
    return bytes;
  }

  /* Captured output. */

  function emit(text, kind) {
    const line = document.createElement('span');
    line.className = kind ? `sq-exec__line is-${kind}` : 'sq-exec__line';
    line.textContent = text;
    out.append(line);
    out.scrollTop = out.scrollHeight;
  }

  /**
   * Routes the snippet's console output into the pane for the duration of a run,
   * and still passes it through to the real console so the devtools view is
   * unchanged. `console.log` is captured rather than replaced by a bespoke
   * print() because logging is what a reader would actually write.
   */
  function captureConsole() {
    const saved = {};
    for (const level of ['log', 'info', 'warn', 'error']) {
      saved[level] = console[level];
      console[level] = (...args) => {
        emit(args.map(format).join(' '), level === 'warn' || level === 'error' ? level : null);
        saved[level].apply(console, args);
      };
    }
    return () => Object.assign(console, saved);
  }

  /** Arrays are the common case here — a quaternion comes back as one. */
  function format(value) {
    if (typeof value === 'string') return value;
    if (typeof value === 'bigint') return `${value}n`;
    if (value instanceof Error) return `${value.name}: ${value.message}`;
    try {
      return JSON.stringify(value, (key, v) =>
        (typeof v === 'number' && !Number.isInteger(v) ? Number(v.toFixed(9)) : v));
    } catch (err) {
      return String(value);
    }
  }

  button.addEventListener('click', async () => {
    button.disabled = true;
    root.classList.add('has-run');
    root.classList.remove('has-error');
    out.textContent = '';
    const started = performance.now();
    const restore = captureConsole();

    try {
      const files = [...MODULE_ASSETS, ...KERNELS];
      const cards = renderCards(files);

      say('Fetching the module and kernels…');
      const bytes = await download(files, cards);

      say('Loading the SpiceQL module…');
      const spiceql = await loadModule();

      // Unfurnish whatever the last run left in the pool, so a re-run is not
      // layered on top of it. KernelSet is a C++ object: GC will not free it.
      if (furnished) { try { furnished.unload(); } catch (err) { /* already gone */ } }

      say('Furnishing kernels…');
      const paths = KERNELS.map((kernel) =>
        spiceql.mountKernel(`/kernels/${kernel.name}`, bytes.get(kernel.name)));
      furnished = new spiceql.KernelSet(paths);

      // What the fence gets. On globalThis because a blob module has no other
      // channel in, and the point is that the snippet does not import a harness.
      globalThis.spiceql = furnishedApi(spiceql);

      say('Running…');
      if (objectUrl) URL.revokeObjectURL(objectUrl);
      objectUrl = URL.createObjectURL(new Blob([code.innerText], { type: 'text/javascript' }));
      await import(objectUrl);

      const secs = ((performance.now() - started) / 1000).toFixed(1);
      const total = [...bytes.values()].reduce((sum, b) => sum + b.length, 0);
      say(`Finished in ${secs}s, with ${paths.length === 1 ? '1 kernel' : `${paths.length} kernels`}`
          + ` furnished out of ${fmtSize(total)} fetched.`);
    } catch (err) {
      // Everything lands here: a failed download, a SPICE error out of the wasm,
      // a syntax error in the edited fence, or anything its top-level await threw.
      console.error(err);
      emit(`${err.name || 'Error'}: ${err.message}`, 'error');
      say(`${err.name || 'Error'}: ${err.message}`, true);
    } finally {
      restore();
      button.disabled = false;
    }
  });
}

document.querySelectorAll('[data-sq-exec]').forEach(setUp);
