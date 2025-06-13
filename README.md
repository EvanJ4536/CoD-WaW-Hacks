# CoD WaW External Memory Patcher

This is an external memory patcher for Call of Duty: World at War Zombies. It uses `WriteProcessMemory` to patch specific in-game instructions and variables at runtime.  

---

## Features

| Feature              | Description                                                                 |
|----------------------|-----------------------------------------------------------------------------|
| God Mode             | Freezes health at 99999 and disables damage logic                           |
| Bottomless Clip      | Disables ammo decrement logic                                               |
| Fly Hack             | Replaces gravity code and lets you float with Space (up) and Shift (down)   |
| All Guns Auto        | Removes delay between shots for automatic fire                              |
| Custom Weapons       | Make any weapon shoot Ray Gun, Wunderwaffe, Panzer rounds instead           |
| Infinite Money       | Sets your in-game money to $1,000,000                                       |
| AI Toggle            | Disables zombie aggression (may interfere with some map interactions)       |

---

## Requirements

- Windows OS  
- C++ compiler

---

## How It Works

- Finds the `CoDWaW.exe` process via `CreateToolhelp32Snapshot`
- Opens a handle to the process with `PROCESS_ALL_ACCESS`
- Uses `WriteProcessMemory` to:
  - Patch instructions (with NOPs or opcodes)
  - Write values directly to memory (e.g., health, money, weapon IDs)

---

## How I Made This Hack  

I used Cheat Engines memory scanner, debugger and disassembler  
  
- I used the memory scanner to find memory addresses that held values for things like player coords, ammo, health, etc.  
- Then I attached a debugger to find out what addresses write to the addresses I found  
- From there it was trial and error, I would go through the list of addresses one at a time and dissassemble and change the instructions at those addresses and see what would happen. maybe nothing, maybe a crash or maybe infinite ammo, etc.  
- Once I found the correct instructions I disassemble the memory region and look for other interesting relevant logic and patch it and see what happens. doing that helped me find things like the current weapon id, zombie aggression instructions, and the gun rechamber timer.  

---

