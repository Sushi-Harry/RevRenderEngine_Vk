# DEVLOG

## 5 July, 2026 [5:27 P.M.]
So I just finished going through the official vulklan tutorial today (Not completely gone through it yet but I'm at the MSAA chapter right now.) Though I haven't finished the tutorial completely, I feel like I'm at a point where I can't really learn anything effectively by just following along to a tutorial and building a learning project contained entirely withing a single main.cpp file consisting of 1500+ lines of code so I'm moving on to my usual approach, essentially throwing myself into the deep end and just learning more stuff as I write a new rendering engine even if I have to rewrite the same project multiple times. Looking forward to it.

## 7 July, 2026 [5:25 P.M.]
Man just setting up large projects is a chore. At least cmake and everything is set up and ready to go. Still trying to swallow the fact that I'll be writing a bit too much boilerplate compared to last time. Really not excited for that. That's it for today I guess. Will try to get the context and window setup finished by tonight before I go to sleep.

## 8 July, 2026 [7:48 P.M.]
Alright so it's just the 3rd day and I'm already starting to get kinda overwhelmed when I think abouot how different Vulkan really is from OpenGL but that's kinda what makes my little brain tingle. It's all very interesting and very verbose. I don't like the verbose part but other than that everything is great. (crying inside tbh)

## 9 July, 2026 [1:18 P.M.]
### _Lost you in the waves, I'm underwater_

## 10 July, 2026 [10:38 P.M.]
yeah so not much to say right now cause this has mostly been a copy paste and make it work kind of exercise so far cause I'm still doing most of the rendering setup stuff which is basically just getting code over from my learning project and porting it over to this one and splitting it into classes for convenience.
The entire thing compiles without any errors though so that's a good thing even though doing this is kinda boring for now. More on this later ig...

## 11 July, 2026 [4:56 P.M.]
I am confused asf right now. Even after splitting everything into classes it's still a bit too much to handle by myself. But that's how I felt when I did a big project in opengl for the first time too so that's noting new. I'll get over it. For now I've added the shader class and its backend implementation. Just need to work on the texture class and its backend implementation and I'm thinking of moving all these object creation functions to vk_utilities.hpp cause they kinda look like unnecessary bloat to my eyes when they're included in the main classes.

## 15 July, 2026 [4:31 P.M.]
Alright so I'm still pretty confused about everything but I'm figuring it out one step at a time. Went on a mini vacation which is why there's a gap between this entry and the previous entry. But I did get some stuff done in the middle of everything. Will work on wiring up everything and finishing the incomplete classes for today.
Alright so it's 10:35 P.M. and I'm writing this to remind my future self to write the DrawIndexed function in the rendering api wrapper.

## 16 July, 2026 [7:20 P.M.]
Man this shit is getting too mangled at this point. But I'll manage it somehow. But the window finally opens and stuff is finally starting to get put together. Window and stuff is working. Just need to add model loading and test texture and shader loading next. So next session is gonna be very fun.

## 17 July, 2026 [5:26 P.M.]
Alright so the screen clearing function works and also added a bunch of stuff that I left empty before. Resizing and stuff works too. It compiles fine and I'd say it's coming along nicely!
Alright so it's __9:10 P.M.__ right now and I've added the layer system from the opengl engine project and also added some window resizing fixes along with the events system that actually manages the different kinds of events like window closing. resizing and stuff. Great progress. And it was actually surprisingly easy to port the code over to this one since the last project's frontend is completely independent of the backend code, meaning that the backend is the only thing that differs in this project instead of literally everything being different. THANK YOU HAZEL. THANK YOU THE CHERNO!!!