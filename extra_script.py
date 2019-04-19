from SCons.Script import DefaultEnvironment
import glob, shutil, os

env = DefaultEnvironment()
# uncomment line below to see environment variables
# print env.Dump()
# print ARGUMENTS

# copyfile("src/WiFiConnector.h", "tmp/WiFiConnector/WiFiConnector.h")
# copyfile("src/WiFiConnector.cpp", "tmp/WiFiConnector/WiFiConnector.cpp")
shutil.rmtree("pio_compile")
os.makedirs("pio_compile")


for file in glob.iglob('src/*.*'):
    print 'Copied file %s' % (file)
    shutil.copy2(file, "pio_compile")

for file in glob.iglob('examples/**/*.*'):
    print 'Copied file %s' % (file)
    shutil.copy2(file, "pio_compile")