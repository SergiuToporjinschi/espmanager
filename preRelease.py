try:
    import configparser
except ImportError:
    import ConfigParser as configparser
import glob, shutil, os, sys

print '########### Run release script ###########'

config = configparser.ConfigParser()
config.read("platformio.ini")

exampleFolder = config.get("common", "examples_folder")
version = config.get("common", "version").replace("\"","")
mainPath = "src/main.cpp"
sourcePath = os.getcwd()

def clearMainFile():
    if os.path.exists(mainPath):
        os.remove(mainPath)

def createExamples():
     shutil.copy2(exampleFolder + "/" + os.path.basename(exampleFolder) + '.ino', 'src/main.cpp')

clearMainFile()
createExamples()

print '########### Finish release script ###########'
