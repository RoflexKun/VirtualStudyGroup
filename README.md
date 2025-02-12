# VirtualStudyGroup (aka StudyBuddy)

## Very small description of the project
&emsp; This project simulates the interaction between a server and multiple students that need a space to share, communicate and collaborate on different projects. I created this project as a little application, with a GUI (created using Qt), a login/register menu and the possibility to check other users who are online the same time as you so you know who is doing the project... or not.

## What tehnologies did I use for this project?
Since this was my, somewhat, biggest project I did until now alone, I was free to use any tehnology to make my life easier. So let's go over the things I used for my project:

  &emsp; -> ***Cmake*** = to be easier to compile a project that uses a graphic library such as Qt and each sub-library of it
  
  &emsp; -> ***Qt graphic library*** = one of the easiest to learn graphic library that helped me achieve a lot of what I wanted to do without needing to write each functionality by hand (helped a lot when re-dimensioned tables)
  
  &emsp; -> ***JSON files*** = needed to save and modify data in a safe and easy manner, such as users, files and other things that needed to be modified in real-time

## What type of server did I use?
&emsp; The most important part to implement in this project is without a discussion: **Sending/Receiving Files**. And for that reason I went with a server implementing a TCP-Protocol, since this protocol was made for this situation, to be able to communicate files without loss.
