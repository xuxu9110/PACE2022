#!/usr/bin/python3
import sys
import os
import json
import csv

args = sys.argv[1:]
dataPath = args[0] if len(args) > 0 else "data"
fileNames = os.listdir(dataPath)
outPath = args[1] if len(args) > 1 else "out.csv"
with open(outPath, "w", newline='') as outFile:
    writer = csv.writer(outFile)
    writer.writerow(["file_name", "num_of_vertex", "num_of_edge", "sum_of_loop", "num_of_round", "init_time", 
        "cooling_time", "feedback_set_size", "is_correct"])
    for fileName in fileNames:
        filePath = dataPath + "\\" + fileName
        os.system(".\\main " + filePath + " " + fileName)
        with open("statistic\\" + fileName, "r") as statisticFile:
            data = json.load(statisticFile)
        data["file_name"] = fileName
        with open(filePath, "r") as file:
            line = file.readline()
            nums = line.split()
            data["number_of_vertex"] = int(nums[0])
            data["number_of_edge"] = int(nums[1])
        result = os.popen(".\\verifier " + filePath + " " + "result\\" + fileName)
        data["is_correct"] = ("OK" in result.read())
        print(fileName + ": " + str(data["is_correct"]))
        writer.writerow([fileName, data["number_of_vertex"], data["number_of_edge"], sum(data["number_of_loop"]), 
            len(data["number_of_loop"]), data["init_time"] / 1e6, data["cooling_time"] / 1e6, 
            data["feedback_set_size"], data["is_correct"]])