# Quad-Tree

I could not find a complete Quad-Tree implementation in C++ so I wrote my own. 
It supports both static and dynamic data, and common spatial queries. 
In addition, the index can be constructed (and updated) according to two different strategies.
The code also provides statistics about the index (e.g., height, size in MB) and about the queries (e.g., execution time, IO cost).

## The implementation supports:
1. Two partitioning methods (center split and optimized point partitioning).
1. Bulk loading.
1. Range queries.
1. K nearest neighbor queries.
1. Dynamic insertions.

## How to compile
    g++ main.cpp QuadTree.cpp Input.cpp

## How to run
    ./quadTree dataFile limit queryFile

* *dataFile*: Denotes the dataset to be bulkloaded. Check *testData.txt* for format.
* *limit*: The number of records to read from dataFile. *-1* bulkloads the entire file.
* *queryFile*: Denotes the file with the queries to be executed. Check *queryTest.txt* for format.

## Partitioning methods
The code supports two splitting strategies for node overflows.
1. *Optimized Point-Quad-Tree*: splits the bucket on the median point.
1. *Point-Region-Quad-Tree*: divides the bucket to equal sized regions.

You can change the partitioning method in *def.h*.

![Alt text](/path/to/QuadTreePointSplit.png "Optimized Point Split")
![Alt text](/path/to/QuadTreeRegionSplit.png "Region Split")

## Index parameters
The index parameters such as capacity and strategy used can be tuned in *def.h*.

*CAPACITY*: sets the maximum number of points in a leaf node. I have only experimented with values greater than *1* (i.e., bucket Quad-Trees).

*POINT_SPLIT*: picks the partitioning strategies described above. If *true*, it uses the optimized point split strategy, while if *false* it creates equal regions.
