/**
 * The REST applet on the home page.
 *
 * Three panes, top to bottom: a log of HTTP requests, the code fence MkDocs
 * highlighted, and the plot. There are no kernels to hold, because the hosted
 * service does the kernel search and the SPK loading and the page receives
 * numbers — the whole reason this applet exists. Its window is two years of
 * Cassini's Saturn tour, 20 000 states, gigabytes of kernels that never leave
 * the server.
 *
 * The harness is deliberately thin. There is no module to load and no kernel to
 * furnish, so the code fence does its own fetching, with its own helper for the
 * response envelope: that helper IS the lesson, and hiding it in the harness
 * would leave a snippet that teaches nothing about the API. All the applet
 * supplies is `plot`, and a fetch it wraps so it can log what went over the wire.
 *
 * Nothing runs or downloads until Run is pressed — not the plotting library, not
 * a single state. Until then the plot pane holds a placeholder instead of 26rem
 * of empty box, and the pane is only given its full height when a run starts
 * (see .has-plot in spiceql-applet.css): Plotly measures its container, so the
 * height has to be there before it draws, not after.
 *
 * The fence is contenteditable and read back out of the DOM at run time, so the
 * example cannot drift from what executes, and every run makes a fresh blob URL
 * so edits take effect.
 */

/** Recognises the fetch failure that means CORS rather than a broken network. */
const CORS_HINT =
  'The API answered, but without an Access-Control-Allow-Origin header, so the'
  + ' browser refused to let this page read it. That header comes from the'
  + ' service (see the CORS middleware in fastapi/app/main.py) and cannot be'
  + ' added by the load balancer in front of it; a deployment carrying it is'
  + ' what this applet needs. curl and pyspiceql are unaffected.';

function setUp(root) {
  const log = root.querySelector('[data-sq-log]');
  const button = root.querySelector('[data-sq-run]');
  const status = root.querySelector('[data-sq-status]');
  const plot = root.querySelector('[data-sq-plot]');
  const code = root.querySelector('.highlight code');
  if (!log || !button || !code) return;

  code.contentEditable = 'true';
  code.spellcheck = false;
  code.setAttribute('aria-label', 'Editable SpiceQL REST example, run with the Run button');

  /** @type {{n:number,endpoint:string,status:string,ms:number,points?:number,spk?:number,bytes?:number}[]} */
  let requests = [];
  let objectUrl = null;

  const say = (text, isError = false) => {
    status.textContent = text;
    root.classList.toggle('has-error', isError);
  };

  const fmtSize = (bytes) =>
    bytes === undefined ? null
      : bytes >= 1048576 ? `${(bytes / 1048576).toFixed(1)} MB`
      : `${Math.max(1, Math.round(bytes / 1024))} KB`;

  /** Points returned so far, across every state call this run has made. */
  const pointsSoFar = () => requests.reduce((sum, r) => sum + (r.points || 0), 0);

  function render() {
    log.textContent = '';
    if (!requests.length) {
      const li = document.createElement('li');
      li.className = 'sq-app__empty';
      li.textContent = 'No requests yet. Run the code and every call it makes is logged here.';
      log.append(li);
      return;
    }

    for (const request of requests) {
      const li = document.createElement('li');
      li.className = 'sq-kcard';
      li.title = request.url;

      const head = document.createElement('span');
      head.className = 'sq-kcard__head';
      const n = document.createElement('span');
      n.className = 'sq-kcard__n';
      n.textContent = String(request.n);
      const name = document.createElement('span');
      name.className = 'sq-kcard__name';
      name.textContent = request.endpoint;
      head.append(n, name);

      const meta = document.createElement('span');
      meta.className = 'sq-kcard__meta';
      const facts = [
        ['status', request.status, true],
        ['time', `${(request.ms / 1000).toFixed(1)}s`],
      ];
      // Two calls to the same endpoint are otherwise indistinguishable in the
      // log — the pair of utcToEt calls that open the example being exactly
      // that, and then eight slices of one window after them.
      if (request.subject) facts.push(['for', request.subject]);
      // Thousands separators: 2500 and 20000 are the numbers that show up here,
      // and they are easier to tell apart grouped than as bare digits.
      if (request.points !== undefined) {
        facts.push(['points', request.points.toLocaleString()]);
      }
      if (request.spk !== undefined) facts.push(['spks used', String(request.spk)]);
      const size = fmtSize(request.bytes);
      if (size) facts.push(['size', size]);

      for (const [label, value, isStatus] of facts) {
        const fact = document.createElement('span');
        fact.className = 'sq-kfact';
        // The status is the one fact worth colouring, and only when it is good —
        // a failure is already being spelled out in the status line below.
        if (isStatus && value === '200') fact.classList.add('is-type');
        const key = document.createElement('i');
        key.textContent = label;
        fact.append(key, document.createTextNode(value));
        meta.append(fact);
      }

      li.append(head, meta);
      log.append(li);
    }
  }

  /** The parameter that says what a request was about, for the log. */
  function subjectOf(url) {
    let params;
    try {
      params = new URL(url, location.href).searchParams;
    } catch (err) {
      return null;
    }
    return params.get('utc') || params.get('target') || params.get('name') || null;
  }

  /**
   * Wraps fetch for the duration of a run so the pane above can show what the
   * snippet actually asked for. Wrapping rather than offering a helper keeps the
   * fence honest: it calls plain fetch, and would behave the same pasted into
   * any page.
   *
   * The response is cloned to count points, which costs one copy of a couple of
   * megabytes and buys the two facts that make the log worth reading — how many
   * states came back, and how many SPKs the service needed to build them.
   */
  function watchFetch() {
    const original = globalThis.fetch;
    globalThis.fetch = async (input, init) => {
      const url = typeof input === 'string' ? input : input.url;
      const entry = {
        n: requests.length + 1,
        url,
        endpoint: url.split('?')[0].split('/').pop() || url,
        subject: subjectOf(url),
        status: '—',
        ms: 0,
      };
      requests.push(entry);
      const started = performance.now();
      try {
        const response = await original(input, init);
        entry.ms = performance.now() - started;
        entry.status = String(response.status);
        const length = response.headers.get('content-length');
        if (length) entry.bytes = Number(length);
        if ((response.headers.get('content-type') || '').includes('json')) {
          try {
            const body = (await response.clone().json())?.body;
            // utcToEt returns a bare number, so `points` is only set for the
            // calls that actually return states and the log omits it elsewhere.
            if (Array.isArray(body?.return)) entry.points = body.return.length;
            if (Array.isArray(body?.kernels?.spk)) entry.spk = body.kernels.spk.length;
          } catch (err) {
            /* not the envelope we expected; the log just omits those facts */
          }
        }
        render();
        return response;
      } catch (err) {
        entry.ms = performance.now() - started;
        entry.status = 'blocked';
        render();
        throw err;
      }
    };
    return () => { globalThis.fetch = original; };
  }

  async function run() {
    button.disabled = true;
    root.classList.remove('has-error');
    // Before the fence runs, not after: this is what gives the pane its height,
    // and Plotly reads that height the moment it draws.
    root.classList.add('has-plot');
    requests = [];
    render();
    const started = performance.now();
    const restore = watchFetch();

    try {
      say('Querying the SpiceQL service… 20 000 states take about six seconds while'
          + ' it loads the SPKs behind them.');
      globalThis.plot = plot;
      if (objectUrl) URL.revokeObjectURL(objectUrl);
      objectUrl = URL.createObjectURL(new Blob([code.innerText], { type: 'text/javascript' }));
      await import(objectUrl);

      const secs = ((performance.now() - started) / 1000).toFixed(1);
      const points = pointsSoFar();
      const spk = requests.reduce((most, r) => Math.max(most, r.spk || 0), 0);
      say(`${points.toLocaleString()} points in ${secs}s over ${requests.length}`
          + ` request${requests.length === 1 ? '' : 's'}`
          + `${spk ? `, built server-side from ${spk} SPKs` : ''}.`
          + ' Nothing was downloaded but numbers. Drag the plot to rotate it.');
    } catch (err) {
      console.error(err);
      // A cross-origin block is indistinguishable from a dead network at the
      // fetch API — both are a bare TypeError with no status — so the message
      // covers the likely cause rather than pretending to know.
      const blocked = requests.some((r) => r.status === 'blocked');
      say(blocked ? `${err.message}. ${CORS_HINT}` : `${err.name || 'Error'}: ${err.message}`,
          true);
      // A run that failed before Plotly drew would otherwise leave the pane at
      // its full height with nothing in it — the blank space this placeholder
      // exists to avoid. Plotly replaces the pane's children when it draws, so
      // a placeholder that is still there means nothing was plotted.
      if (plot.querySelector('.sq-app__placeholder')) root.classList.remove('has-plot');
    } finally {
      restore();
      button.disabled = false;
    }
  }

  button.addEventListener('click', run);

  render();
}

document.querySelectorAll('[data-sq-rest]').forEach(setUp);
