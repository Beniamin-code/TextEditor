# Minimalist C++ Text Editor

> A lightweight and efficient text editor core built in C++, optimized for handling large files through the **Piece Table** data structure.

---

## 🚀 Key Features

* **Efficient Core Engine:** Powered by a Piece Table architecture for consistent performance regardless of file size.
* **Standard Operations:** Full support for typing, deleting, cutting, copying, and pasting text.
* **Configurable Undo/Redo:** A robust state management system with a customizable history limit for navigating edit history.
* **Native Clipboard Integration:** Seamless interaction with the system clipboard (System-wide Copy/Paste) for a smooth workflow.
* **File Persistence:** Reliable Save and Open functionalities for local file management.
* **Precision Keyboard Selection**: Custom text selection logic using Shift + Arrow keys, designed for granular control over text blocks.
* **Keyboard-Driven Workflow**: Optimized for speed with support for industry-standard hotkeys (Ctrl+C, Ctrl+V, Ctrl+Z, etc.).

---

## 🛠️ Tech Stack

* **Language:** C++ (Standard ISO/IEC 14882:2017 or later)
* **Architecture:** Piece Table (Buffer-based management)
* **System Integration:** Win32 API (Native Windows Clipboard management)
* **Toolchain:** MSVC (Microsoft Visual C++), Visual Studio 2022
---

## 🧑‍💻 My Contribution: Core Engine & Algorithms

In this collaborative project, I was responsible for the architectural design of the editor's core and the underlying algorithmic logic.

**Key Technical Achievements:**

* **Piece Table Implementation:** Designed and implemented the primary data structure in C++. This approach reduced editing complexity from O(n) (linear buffers) to O(k) (where k is the number of pieces), ensuring optimal memory usage and high-speed performance even with large documents.
* **Configurable Undo/Redo System:** Developed a state-tracking manager based on piece history, allowing the user to revert or redo actions. I designed the system to support a configurable history limit, ensuring efficient memory management and flexibility.
* **Native Clipboard Integration:** Engineered the bridge between the editor and the **Win32 API** to enable system-wide Copy/Paste functionality, allowing data transfer between this editor and other applications.
* **Memory Optimization (Zero-copy approach):** Optimized text fragment handling to minimize unnecessary allocations and string copying, focusing on high-performance memory management.

---

## ⌨️ Controls

* **Arrow Keys:** Move cursor
* **Shift + Arrows:** Select text
* **Ctrl + C / V / X:** Copy / Paste / Cut
* **Ctrl + Z / Y:** Undo / Redo
* **Ctrl + S / O:** Save / Open

---

## ⚙️ Installation & Usage

### Prerequisites
* Windows OS (Required for Win32 API)
* A C++17 compatible compiler (e.g., MSVC or MinGW)

1. **Clone the repository:**
   ```bash
   git clone [https://github.com/Beniamin-code/TextEditor.git](https://github.com/Beniamin-code/TextEditor.git)
   ```

2. **Build the project:**
   ```bash
   g++ *.cpp -o TextEditor.exe -std=c++17 -O3
   ```

3. **Run the editor:**
   ```bash
   ./TextEditor
   ```

📝 License
This project is licensed under the MIT License. See the LICENSE file for details.
Copyright (c) 2026 Ursaciuc Sebastian and Merticariu Beniamin.

🤝 Contact
Merticariu Beniamin * LinkedIn: https://www.linkedin.com/in/beniamin-merticariu-34267738b/

Email: beniaminmerticariup@gmail.com









