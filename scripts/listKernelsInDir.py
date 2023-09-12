from glob import glob

import re

if __name__ == "__main__":
  term = re.compile(".*(bc|ti|tf|bsp|tsc)", re.IGNORECASE)
  files = glob("/usgs/cpkgs/isis3/isis_data/*/kernels/*/*")
  found = []

  for f in files: 
      if re.match(term, f):
          found.append(f)
          
  with open("isisdata.txt", "w") as f:
      f.write("\n".join(found))