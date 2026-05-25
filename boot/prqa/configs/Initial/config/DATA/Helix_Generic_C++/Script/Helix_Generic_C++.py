import os, sys
from pathlib import Path

def sysInclude(myCct):
    try:
        # walk through stub directory and add them to cip
        cipFile = open(myCct.cipFilePath(), 'w')
        stubList = [os.path.join(d, t) for d, ds, fs in os.walk(myCct.stubDir()) for t in ds]
        for name in stubList:
            cipFile.write('-si "' + name + '"\n')
            cipFile.write('-q "' + name + '"\n')
        for fileName in stubList:
            if fileName.endswith("forceinclude"):
                fileList = [os.path.join(fileName, x) for x in myCct.getForceIncludes(fileName)]
                for fn in fileList:
                    cipFile.write('-fi "' + fn + '"\n')
        cipFile.close()

    except IOError:
        myCct.exit_error(1, 'Error writing to CIP', cipFile)

if __name__ == "__main__":
    qafDir = str(Path(__file__).parent.joinpath('..', '..', 'core_qaf').resolve())
    if qafDir not in sys.path:
        sys.path.insert(0, qafDir)

    try:
        import qaf
        myCct = qaf.Cip(str(Path(__file__).resolve()))
        sysInclude(myCct)
    except ModuleNotFoundError:
        print('Unable to locate "qaf.py" module. Exiting.')
        sys.exit(1)
