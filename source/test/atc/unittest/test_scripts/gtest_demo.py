import os
import time
import sys
import xml.etree.ElementTree as ET
# A simply example to explain how to do the unit test.

if len(sys.argv) == 2:
    project = sys.argv[1]
else:
    print('You need specify the project!')

print('test start')

cmd = 'adb wait-for-device'
print(cmd)
res = os.system(cmd)
if res != 0:
    exit(res)

cmd = 'adb shell "cat /proc/cmdline"'
print(cmd)
result = os.popen(cmd).readlines()
print(result)

if result and 'mtdparts=atcnand' in result[0]:
    print('deivce is nand, start disable eMMC write protect.')
    cmd = 'adb shell "echo clear_all_wp > /proc/nand_wp"'
    print(cmd)
    res = os.system(cmd)
    if res != 0:
        exit(res)
else:
    print('deivce is emmc, start disable eMMC write protect.')
    cmd = 'adb shell "echo 0x19 0x1 0x55 > /proc/msdc_debug"'
    print(cmd)
    res = os.system(cmd)
    if res != 0:
        exit(res)

    cmd = 'adb reboot'
    print(cmd)
    res = os.system(cmd)
    if res != 0:
        exit(res)

cmd = 'adb wait-for-device'
print(cmd)
res = os.system(cmd)
if res != 0:
    exit(res)

cmd = 'adb remount'
print(cmd)
res = os.system(cmd)
if res != 0:
    exit(res)

# push bin to the device
print("push bin to the device...")
cmd = 'adb push test_suite/unittest/bin/demo_unittest /data '
print(cmd)
res = os.system(cmd)
if res != 0:
    exit(res)

cmd = 'adb shell "chmod -R 777 /data/demo_unittest"'
print(cmd)
res = os.system(cmd)
if res != 0:
    exit(res)

cmd = 'adb shell "mkdir -p /data/unittest/demo_unittest"'
print(cmd)
res = os.system(cmd)

if not os.path.exists("result"):
    os.mkdir("result")
if not os.path.exists("log"):
    os.mkdir("log")

cmd = 'adb shell "/data/demo_unittest --gtest_output=xml:/data/unittest/demo_unittest/GTest_Result_Demo_Test.xml" >log/demo_unittest.log 2>&1'
print(cmd)
res = os.system(cmd)

cmd = 'adb pull /data/unittest/demo_unittest result/'
print(cmd)
res = os.system(cmd)
if res != 0:
    exit(res)

#check result from gtest result xml
failures = 0
errors = 0

resultxmlfilepath = os.path.abspath("result/demo_unittest/GTest_Result_Demo_Test.xml")
tree = ET.ElementTree(file=resultxmlfilepath)
root=tree.getroot()

for testsuites in root.iter("testsuites"):
    failures=testsuites.attrib['failures']
    errors=testsuites.attrib['errors']

print("failures testcases is", failures)
print("errors testcases is", errors)

if failures == '0' and errors == '0':
    print('++++++++++++PASS+++++++++++++++')

else:
    print('++++++++++++Fail+++++++++++++++')

exit(0)
