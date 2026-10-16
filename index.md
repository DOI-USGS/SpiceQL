This library provides a C++ interface querying, reading and writing NAIF SPICE kernels. Built on the [NAIF Toolkit](https://naif.jpl.nasa.gov/naif/toolkit.html).

## Try it over the REST API

The plot below is **two years of Cassini's Saturn tour**: 20 000 states that the [hosted service](https://astrogeology.usgs.gov/apis/spiceql/latest/docs) builds out of 37 SPKs. The kernels stay on the server: gigabytes that would never be worth shipping to a page, and all that comes back is numbers.

Nothing is fetched until you press **Run**: not the plotting library, not a state. Edit the code and run it again as often as you like; the pane above the code logs every request the snippet makes, with the points each one returned.

See [API docs](https://astrogeology.usgs.gov/docs/getting-started/using-spiceql/exploring-spiceqls-rest-python-and-cpp-apis/#__tabbed_3_1) for examples of SpiceQL functions.

Requests Every HTTP call the code makes, in order, with what came back, including the points each one returned. Two epoch conversions, then one ranged call that answers with all 20 000 states.

```
import Plotly from 'https://cdn.jsdelivr.net/npm/plotly.js-gl3d-dist-min/+esm';

// Every endpoint answers {statusCode, body: {return, kernels}}, so one helper
// covers the whole API. `plot` is the element below.
const API = 'https://astrogeology.usgs.gov/apis/spiceql/latest';

async function spiceql(endpoint, params) {
  const response = await fetch(`${API}/${endpoint}?${new URLSearchParams(params)}`);
  if (!response.ok) throw new Error(`${endpoint}: HTTP ${response.status}`);
  // SPICE failures come back INSIDE an HTTP 200 as {statusCode: 500, body:
  // {error}}; so the envelope is what says whether this worked. Skip this and
  // a failed call returns a body with no `return`, and the first thing you do
  // with it throws something unrelated several lines later.
  const { statusCode, body } = await response.json();
  if (statusCode !== 200) throw new Error(`${endpoint}: ${body?.error ?? statusCode}`);
  return body;
}

// Call utcToEt on datetimes in range. getTargetStatesRanged can fail if there is a hole 
// anywhere in a step between start stop time window.  
const { return: etStart } = await spiceql('utcToEt', { utc: '2008-07-01T00:00:00' });
const { return: etStop }  = await spiceql('utcToEt', { utc: '2010-09-14T00:00:00' });

// getTargetStatesRanged calls a vecotized routine. Far more effecient that calling for 
// ETs indivdidualy as it minimizes calls.  
const { return: states, kernels } = await spiceql('getTargetStatesRanged', {
  startEt: etStart,
  stopEt: etStop,
  numRecords: 20000,
  target: 'Cassini',
  observer: 'SATURN BARYCENTER',
  frame: 'J2000',
  abcorr: 'None',
  mission: 'cassini',
  limitSpk: -1,
  limitCk: -1,
  searchKernels: true,
});
console.log(`${states.length} states from ${kernels.spk.length} SPKs`);

// plot code 
const days = states.map((_, i) => (etStart + (i * (etStop - etStart)) / (states.length - 1)));

Plotly.newPlot(plot, [{
  type: 'scatter3d', mode: 'lines', name: 'Cassini',
  x: states.map(s => s[0]), y: states.map(s => s[1]), z: states.map(s => s[2]),
  line: { width: 2, color: days, colorscale: 'YlOrRd', reversescale: true },
}, {
  type: 'scatter3d', mode: 'markers', name: 'Saturn barycentre',
  x: [0], y: [0], z: [0], marker: { size: 4, color: '#E8C36A' },
}], {
  margin: { l: 10, r: 10, t: 10, b: 10 },
  showlegend: false,
  paper_bgcolor: 'rgba(0,0,0,0)',
  scene: {
    // One kilometre is the same length on all three axes, so the orbit keeps
    // its shape instead of being stretched to fill the box.
    aspectmode: 'data',
    camera: { eye: { x: 1.6, y: 1.6, z: 1.1 } },
    xaxis: { title: 'X (km)' }, yaxis: { title: 'Y (km)' }, zaxis: { title: 'Z (km)' },
  },
});
```

Run

Nothing to download but numbers; the kernels stay on the server. About 1.7 MB for the plotting library, then 2.6 MB for 20 000 states.

Press **Run** to plot 20 000 Cassini states, fetched live from the service. Nothing is downloaded until you do.

## Getting Started

You can install using [Anaconda](https://www.anaconda.com/download/success?reg=skipped) or [miniforge](https://github.com/conda-forge/miniforge).

```
conda install -c conda-forge spiceql
```

To create the initial database, set your required environment variables.

```
# Path to some directory with kernels. SpiceQL searches for them recursively
export SPICEROOT="path/to/spice/kernels/"
# anywhere you have write permissions to
export SPICEQL_CACHE_DIR="path/to/cache/"
```

Run `create_database()`; this is more easily done through python.

Warning

This might take several hours depending on the number of kernels in your folder

```
SPICEQL_LOG_LEVEL=INFO python -c "import pyspiceql; pyspiceql.create_database()"
```

### Basic usage

```
import pyspiceql as psql 

# search for a kernel set
kernels = psql.searchForKernelsets(["odyssey", "mars"], ["sclk", "spk", "tspk", "ck"], 715662878.32324, 715663065.2303)
print(kernels)


# Make a query and it will return the kernels used 
orientations, kernels = psql.getTargetOrientations([690201375.8323615], -74000, -74690, "ctx", searchKernels=True) 
print(kernels)
print(orientations)
```

```
#include <spiceql/spiceql.h>
#include <nlohmann/json.hpp>

// search for a kernel set
nlohmann::json kernels1 = SpiceQL::searchForKernelsets({"odyssey", "mars"}, {"sclk", "spk", "tspk", "ck"}, 715662878.32324, 715663065.2303);
std::cout << kernels1.dump(2) << std::endl;

// Make a query and it will return the kernels used 
auto [orientations, kernels2] = SpiceQL::getTargetOrientations({690201375.8323615}, -74000, -74690, "ctx", {"smithed", "reconstructed"}, false, true);
std::cout << kernels2.dump(2) << std::endl;
// nx4 array of quaternions 
std::cout << orientations.size() << std::endl;
```

SpiceQL compiles to WebAssembly, so the same `api.h` calls run in the browser through Embind. Two differences shape the example. The browser build ships no HDF5 inventory, so **there is no kernel search**, `searchForKernelsets` and `searchKernels: true` both throw, and kernels are furnished explicitly. And the WASM signatures take no `mission` argument since there is no search.

Fetching and furnishing those kernels is the harness's job, not the example's, so it happens in the pane above the code and `spiceql` arrives already loaded. Press **Run**; nothing downloads until you do, and the browser caches it, so a second run is instant.

Downloads The module and the kernels this example furnishes, fetched on Run and then served from the browser cache. One 115 KB frame kernel is all it needs: the CTX-to-bus rotation is a fixed mounting offset written down in the FK, so no CK has to be loaded to read it.

```
const { result: orientations } = spiceql.getTargetOrientations(
  [690201375.8323615], -74000, -74690);

const [w, x, y, z, ...angularVelocity] = orientations[0];
console.log('quaternion:', [w, x, y, z]);
console.log('angular velocity (rad/s):', angularVelocity);

const degrees = 2 * Math.acos(Math.min(1, Math.abs(w))) * 180 / Math.PI;
console.log(`a fixed rotation of ${degrees.toFixed(4)} degrees`);

// The one api.h feature the browser build does not have.
try {
  spiceql.getTargetOrientations([690201375.8323615], -74000, -74690,
                                { searchKernels: true });
} catch (err) {
  console.log('searchKernels: true ->', err.message);
}
```

Run

Nothing runs until you press Run. The first run fetches about 3 MB: the WASM module, its config DB, and the one kernel above.

### Online Interface

Some functions allow for running over the web; these contain the optional parameter `useWeb`. See the [function list](https://doi-usgs.github.io/SpiceQL/SpiceQLCPPAPI/namespace_spice_q_l.md) for a list of functions with this parameter.

```
import pyspiceql as psql 

# Make a query and it will return the kernels used 
orientations, kernels = psql.getTargetOrientations([690201375.8323615], -74000, -74690, "ctx", useWeb=True) 
print(kernels)
print(orientations)
```

```
#include <spiceql/spiceql.h>
#include <nlohmann/json.hpp>

// Make a query and it will return the kernels used 
auto [orientations, kernels] = SpiceQL::getTargetOrientations({690201375.8323615}, -74000, -74690, "ctx", {"smithed", "reconstructed"}, true);
std::cout << kernels.dump(2) << std::endl;
// nx4 array of quaternions 
std::cout << orientations.size() << std::endl;
```

## Where to get NAIF kernels

`downloadIsisData.py` is a script that downloads kernels from NAIF and USGS sources. It includes a SpiceQL database.

If you have ISIS, you may have already set up the data area. If you don't have ISIS, you will need to download the `rclone` program, the `downloadIsisData` script, and ISIS's `rclone.conf`. See [Downloading ISISDATA](https://astrogeology.usgs.gov/docs/how-to-guides/environment-setup-and-maintenance/isis-data-area/#downloading-isisdata) for more info on the download script, or install as below if you don't have ISIS:

```
# Install rclone 
mamba install rclone

# Download the script and rclone config file
curl -LJO https://github.com/USGS-Astrogeology/ISIS3/raw/dev/isis/scripts/downloadIsisData
curl -LJO https://github.com/USGS-Astrogeology/ISIS3/raw/dev/isis/config/rclone.conf

# Use python 3 when you run the script,
# and use --config to point to where you downloaded the config file 
python3 downloadIsisData --config rclone.conf <mission> $SPICEROOT
# set your cache dir to the same as SPICEROOT
export SPICEQL_CACHE_DIR=$SPICEROOT
```

Download data from [NAIF](https://naif.jpl.nasa.gov/naif/data_archived.html)

You can download using `wget` or a similar application.

## Bots

These docs ship with [llms.txt](https://doi-usgs.github.io/SpiceQL/llms.txt) and [llms-full.txt](https://doi-usgs.github.io/SpiceQL/llms-full.txt) for bots to read for learning the library.
