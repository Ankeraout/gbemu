# Expected command line:
# python runtests.py <path to executable> <path_to_test_data>

import sys
import json
import os.path
import subprocess

def runtest(executable, test_data):
    print('Running test')

def printUsage():
    print('Usage:')
    print('python' + sys.argv[0] + ' <path to executable> <path to test data>')

def checkIndividualTest(individualTestData):
    if(type(individualTestData) != dict):
        print('Individual test is not a dictionary.')
        return 1
    
    if('expectedOutput' not in individualTestData.keys()):
        print('Missing "expectedOutput" key in individual test.')
        return 1

    if('romFile' not in individualTestData.keys()):
        print('Missing "romFile" key in individual test.')
        return 1

    if('maxClocks' not in individualTestData.keys()):
        print('Missing "maxClocks" key in individual test.')
        return 1

    return 0

def checkTestData(testData):
    if(type(testData) != list):
        print('Test data is not an array.')
        return 1
    
    for individualTestData in testData:
        if(checkIndividualTest(individualTestData) != 0):
            return 1

    return 0

def run_test(executable, testData):
    returnValue = subprocess.Popen(
        [
            executable,
            '--redirect-serial-file', 'tmp.bin',
            '--disable-fps-limit',
            '--disable-graphics',
            '--max-clocks', str(testData['maxClocks']),
            '--bios', 'roms/gbbios.bin',
            '--rom', testData['romFile'],
            '--disable-sram-load',
            '--disable-sram-save',
        ]
    ).wait()

    failed = False

    if(returnValue != 0):
        return 'Invalid return value.'
    
    try:
        serialOutputFile = open('tmp.bin', 'rb')
    except:
        return 'Could not open tmp.bin.'
    
    serialOutputData = serialOutputFile.read()

    serialOutputFile.close()

    if(len(testData['expectedOutput']) != len(serialOutputData)):
        return 'Serial output data comparison failed.'

    index = 0

    for character in testData['expectedOutput']:
        characterOrd = ord(character)

        if(characterOrd != serialOutputData[index]):
            return 'Serial output data comparison failed.'

        index += 1

    return 'Passed'

def run_tests(executable, testData):
    index = 1
    passed = 0

    for individualTestData in testData:
        print('Running test ' + str(index) + ' of ' + str(len(testData)) + ' (' + individualTestData['romFile'] + ')... ', end='')

        result = run_test(executable, individualTestData)

        if(result == 'Passed'):
            passed += 1

        print(result)
        index += 1

    print()
    print('===== Test results =====')
    print('Tests run   : ' + str(len(testData)))
    print('Tests passed: ' + str(passed))
    print('Tests failed: ' + str(len(testData) - passed))

def main(args):
    if(len(sys.argv) != 3):
        printUsage()
        exit(1)
    
    executable = sys.argv[1]
    testDataFileName = sys.argv[2]

    if(not os.path.exists(executable)):
        print('Executable file does not exist.')
        exit(1)

    # Read test data
    try:
        testDataFile = open(testDataFileName, 'r')
    except:
        print('Failed to open test data file.')
        exit(1)
    
    testData = json.load(testDataFile)

    testDataFile.close()

    # Check test data
    if(checkTestData(testData) != 0):
        print('Invalid test data.')
        exit(1)

    run_tests(executable, testData)

main(sys.argv)
