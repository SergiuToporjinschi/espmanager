import shutil, os
try:
    import configparser
except ImportError:
    import ConfigParser as configparser

print('########### Run addMainToSrc ###########')

config = configparser.ConfigParser()
config.read("platformio.ini")


exampleFolder = config.get("common", "examples_folder")
mainPath = "src/main.cpp"

if os.path.exists(mainPath):
    os.remove(mainPath)

shutil.copy2(exampleFolder + "/" + os.path.basename(exampleFolder) + '.ino', mainPath)


print('########### Finish addMainToSrc ###########')
