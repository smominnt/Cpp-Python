# Cpp-Python
Compilers/Programming Language Paradigms project Fall 2018

Shafi Momin

B. Barron

----------------------------------------
compile with:

g++ -std=c++11 -lboost_regex -o Source main.cpp eval.cpp

Execute with:

./Source filename.py

example: 

./Source input.py

---------------------------------------
If/else checking: 
The program does a simple count of the number of ifs and elses to see whether or not it can be valid. There is also an additional check step during the nesting level calculation. This uses boolean variables to see if an else statement directly follows an if statement without interruption. Otherwise, the if-else statements are considered invalid.

If/else nesting:
The program will scan line by line until an if statement is met, after which it passes to the function which will recursively evaluate each depth. After the evaluation is complete for each body, a skip function is called which makes the iterator jump to the line right after the statement ends. In the case of an if/else pair, the deepest of the two is returned. The program is designed to be able to go more than just level 3.

Mutation checking:
The program will scan and add any variable to the map that does not have any indentation. If there is indendation, it is assumed that said variable is localized. If the same variable appears again, it is marked as mutated. In the case that a localized variable is found within an if/else statement and is mutating an existing global variable, it is marked as mutated. All variables that are modified within functions are completely ignored as they are localized. 

Recursion checking: The recursive check function assumes that the recursive function mutates the function variable in the return statement that calls the function again. It should be able to detect if recursive function ends depending on the if statement and if the mutated variable in the returned function call actually mutates to able to meet the if statement's requirement.

NOTE: The program uses a different output style for clarity on which function ends recursion.
