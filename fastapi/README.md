# SpiceQL FastAPI App

## Create local instance

### 1. Create conda environment
Create the conda environment to run your local instance in:
```
conda env update -n spiceql-api -f environment.yaml
```

### 2. Set environment variables
Similarly to your SpiceQL conda environment, set `SPICEROOT` or `ISISDATA` to your ISIS data area. You may also need to set `SSPICE_DEBUG` to any value, like `True`.

To set an environment variable within the scope of your conda environment:
```
conda activate spiceql-api
conda env config vars set SPICEROOT=/path/to/isis_data
```

### 3. Run the app
Within the `fastapi/` dir but outside the `app/` dir, run the following command:
```
uvicorn app.main:app --reload --port 8080
```

You can access the Swagger UI of all the endpoints at http://127.0.0.1:8080/docs.
