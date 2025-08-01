import logging
import sys


logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger("spiceql")
handler = logging.StreamHandler()
formatter = logging.Formatter('SPICEQL [%(asctime)s] [%(levelname)s] [%(filename)s:%(lineno)d %(funcName)s] %(message)s', datefmt='%H:%M:%S %z')
handler.setFormatter(formatter)
logger.addHandler(handler)
logger.propagate = False

logger.setLevel(getattr(logging, "DEBUG", logging.DEBUG))

