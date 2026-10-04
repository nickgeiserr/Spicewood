# Aegis

## Exit Codes

(Guess) **0x0000000A** = Generic Crash  
(Guess) **0x00001002** = Memory Modification 


## What's it doing?

### Confirmed Crashes :
- Console Window

Trying to figure that out. I'm running tests by line-by-line rebuilding my framework, catching where it detects and crashes, and reimplementing it. 

#### Test 1
So far, it hasn't crashed on a normal injected empty dll (breaks on attach.) Process explorer confirms the dll is in the game. It's not detecting that nor monitoring createremotethread etc.

#### Test 2
It didn't crash when I made that same empty dll create a thread that just runs a while loop that sleeps and runs forever. Process explorer confirmed the dll was in and the thread was running.

#### Test 3
I added a console window with a simple debug message and it crashed. The game is detecting this. I'm going to try and write directly to an output file. 

#### Test 4
Instead of opening a console and writing to it, I made it write to a local file called debug_log.txt. The game did not crash.

#### Test 5
I'm working on changing my initial modding library to use a pipe instead of opening a console. let's see how that goes! It works. my original framework works completely fine without issue when I switch to the pipe for debugging and information. 

#### Test 6 
I'm going to try and re-enable my hooking code and see if it still crashes all the same. This one would not surprise me at all. it crashed. but also my current code uses a dictionary to kinda spam print every new function.

#### Test 7
im just gonna switch it to hooking process event and then whenever its called, run the original processevent and return. just to see if the hook existing is crashing it. crashed. 

#### Test 8
I'm going to try and instead of inline hooking process event, im going to go to the UWorld vmt and modify the ProcessEvent pointer to my own, then pass it back to the real one.

