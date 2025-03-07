# VirtualStudyGroup (aka StudyBuddy)

## Table of Contents
- [Project Overview](#project-overview)
- [Technologies Used](#technologies-used)
- [Communication Protocol](#communication-protocol)
- [Features](#features)
  - [Server Connection & Login](#server-connection--login)
  - [Main Menu](#main-menu)
  - [Chat Room](#chat-room)
  - [File Management](#file-management)
  - [Live Collaboration](#live-collaboration)
- [Disclaimer](#disclaimer)

---

## Project Overview
This project simulates the interaction between a server and multiple students who need a space to share, communicate, and collaborate on different projects. It features a **Graphical User Interface (GUI)** built with Qt, a **login/register system**, and a way to see other users who are online.

---

## Technologies Used
Since this was my biggest solo project so far, I chose technologies that made development easier:

- **CMake** – Simplifies compiling a project that uses a graphical library like Qt.
- **Qt (Graphic Library)** – A user-friendly GUI library that streamlined development, especially for UI resizing.
- **JSON Files** – Used for safely storing and modifying real-time data such as users and shared files.

---

## Communication Protocol
The most crucial feature of this project is **sending and receiving files**. To ensure reliable communication without data loss, the server implements a **TCP protocol**, which is designed specifically for such scenarios.

---

## Features

### Server Connection & Login
Before accessing the application, users must **connect to the server**. The server acts as the backend, processing user requests and returning results. After connecting, users must **log in or register** to interact with others.

<p align="center">
  <img src="https://github.com/user-attachments/assets/9a214578-775d-43b7-95b4-bea4e3c11ead" width="400" />
</p>
<p align="center">Fig. 1: Server Connection Menu</p>

<p align="center">
  <img src="https://github.com/user-attachments/assets/237c36ce-e584-40c3-a407-a2758893fa18" width="400" />
</p>
<p align="center">Fig. 2: Login Menu</p>

<p align="center">
  <img src="https://github.com/user-attachments/assets/f165dd22-a484-4f3b-a9fd-79b8a82cdfa1" width="400" />
</p>
<p align="center">Fig. 3: Registration Menu</p>

### Main Menu
After logging in, users see the **main menu**, which provides access to the core functionalities.

<p align="center">
  <img src="https://github.com/user-attachments/assets/9d44d6a2-285f-4844-ad8a-9f8745d6f1fc" width="400" />
</p>
<p align="center">Fig. 4: Main Menu</p>

### Chat Room
The chat system consists of three main sections:
- **Active Users** – Displays currently online users in real-time.
- **Chat Log** – Shows past messages.
- **Send a Message** – Allows users to send messages, which are stored in a JSON file.

<p align="center">
  <img src="https://github.com/user-attachments/assets/05b5a074-d1c1-47d0-a343-06f824c580f1" width="400" />
</p>
<p align="center">Fig. 5: Chat Room Interface</p>

### File Management
Users can upload and download files from a **cloud-like system** hosted on the server.

- **To download a file:** Click on the file and press the download button. The file will be saved in the `Downloads` folder.
- **To upload a file:** Click the upload button and enter the file path.

<p align="center">
  <img src="https://github.com/user-attachments/assets/d3bc632f-3e5d-4c3d-adf7-88b7ab3bc495" width="400" />
</p>
<p align="center">Fig. 6: Downloading a File</p>

<p align="center">
  <img src="https://github.com/user-attachments/assets/55ab58b1-7ca3-4b27-8710-2c34da1dcd78" width="400" />
</p>
<p align="center">Fig. 7: Uploading a File</p>

### Live Collaboration
This feature was initially meant to support **real-time file modifications**, but due to bugs and time constraints, it was revised. Now, users can:

- **See who is working on the same file.**
- **Receive notifications** when modifications are made.
- **Sync (Pull)** the latest version from the server.
- **Push (Upload)** a modified version to the server (similar to Git's push/pull system).

<p align="center">
  <img src="https://github.com/user-attachments/assets/2c5b1fd7-8e18-4389-b1f8-e3779c030519" width="400" />
</p>
<p align="center">Fig. 8: Live Collaboration Interface</p>

---

## Disclaimer
⚠️ **Important:** This project was initially designed to run on my local machine. Paths are hardcoded for my setup, so modifications may be needed to run it on other machines.

📝 This project was developed for the **Computer Networks** course at UAIC Faculty of Computer Science.
