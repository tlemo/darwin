
import os

def reserve_universe(filename):
    """deletes the file if it exists, then returns the full path"""
    darwin_home_path = os.getenv('DARWIN_HOME_PATH')
    if darwin_home_path is None:
        raise RuntimeError('DARWIN_HOME_PATH must be set before running the tests')
    full_path = os.path.abspath(os.path.join(darwin_home_path, filename))
    if os.path.exists(full_path):
        os.remove(full_path)
    return full_path
