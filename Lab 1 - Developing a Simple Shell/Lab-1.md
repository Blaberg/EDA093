#Lab 1 - Developing a Simple Shell

This assignment will give you a chance to warm up your C programming skills,
see what the UNIX operating system offers in terms of system calls, and also
let you implement one of the most important system applications: a command shell. 
Your shell must be able to run programs on Linux platforms, and must support features 
such as basic IO redirection and pipes. 

##Getting Started
 - Join a project group.
 - Watch the introduction to the lab.
 - Pass the Lab 1 Preparation Test, to familiarize yourself with some basic prerequisites 
 before you can efficiently work on Lab 1.
 Passing the quiz unlocks the submission page. 
 Check out the appendix of the lab description if you get stuck!
 - Download the skeleton code lab1.zip and read the Lab 1 instructionsPreview the document carefully.
 - Develop your shell and write your report. If you have any questions, check the FAQs below. Maybe your question has 
 already been answered!
 - Follow the detailed submission instructions in the lab PDF to submit your work.
 There is no possibility for resubmission, so test your solution thoroughly!

##FAQ
###What is the expected behavior of the grep apa | ls command?
This command checks if processes spawned by the custom shell wait correctly for the termination of their children when using pipes.

About grep

First of all, consider the first part, grep apa 

As described in the grep man entry, "grep searches the named input FILEs (or standard input if no files are named) for 
lines containing a match to the given PATTERN. By default, grep prints the matching lines." 

Since we only provide a PATTERN argument (apa), grep will read the standard input, searching for our pattern. It will 
end the search when it reads the EOF character. You can insert this character is by pressing <Ctrl - D>. You can verify 
this behavior by running grep apaand then typing various lines of text, some of which contain the "apa" pattern.  The lines containing the pattern will be (re)printed by grep. The process is terminated when you insert the EOF character. 

Waiting grep + pipe

If your custom shell is working correctly, grep should still wait for the EOF even if you pipe its output to some other 
command such as ls. The output of the ls command might be printed either before or after grep is terminated (in bash it comes before), but ideally grep should wait until it receives the EOF. Returning to the shell prompt immediately and leaving grep running (and reading input) might lead make your shell behave strangely (e.g. ignore typed characters) or even completely unusable. Such solutions will not be accepted.