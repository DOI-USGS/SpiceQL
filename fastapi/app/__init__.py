import logging
import os

log_level = os.environ.get('SPICEQL_LOG_LEVEL')
if log_level is not None and log_level.lower() == "debug":
    log_level = logging.DEBUG
else:
    log_level = logging.INFO

logging.basicConfig(
    level=log_level,
    format='SpiceQL-FastAPI [%(asctime)s] [%(levelname)s] [%(filename)s:%(lineno)d %(funcName)s] %(message)s',
    datefmt='%H:%M:%S %z')
