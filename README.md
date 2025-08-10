# C++ Wave Function Collapse

https://github.com/user-attachments/assets/fa13506b-055c-48aa-a1c7-87ed2e0f6f4f

**Dennis Chase**

Aug 2022

## Summary

The purpose of this program was to learn about **Wave Function Collapse**. 
It's all ran in the console, using box/line-drawing characters (which I refer to as pieces). 

Some terms that may need a little intro:

> **Kernal**: a tile on the map

> **Entropy**: a list of valid pieces for the individual **Kernal**

> **Collapsed**: when a **Kernal**'s  **Entropy** is reduced to a single option.


The idea is to fill out an area in a way that all adjacent **Kernals** are connected in a smooth way.
Whe way I tackled this is by starting with a board of **Kernals** that each start with an **Entropy** that has all possible pieces.
Then it goes through, collects all the **Kernals** that have the same lowest matching **Entropy** count (the first time it includes all the **Kernals**).
Then from the collected, a random **Kernal** is selected, then from that **Kernal**, a random piece is chosen from its **Entropy**. 
That tile is now **Collapsed**, and the **Entropy** is updated on all **Kernals** on which tiles can now connect properly.
This can affect more than the immediately adjacent **Kernals**, so I recursively update the **Entropy**.
This happens until all **Kernals** are **Collapsed** and the board is filled out completely.

## Running the Program

When running the program, enter the height and width of the map you'd like to generate.
Next, choose if you want to see the board *animate* by showing the generation of one piece at a time. 
If you want to, then choose the speed.
