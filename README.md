Steganography (C Project)

The Steganography Project is a C-based application that hides secret text messages within image files using the Least Significant Bit (LSB) technique. It demonstrates the integration of file handling, bitwise operations, and data encoding/decoding concepts.

🚀 Features

Encodes secret text into a .bmp image file without noticeable visual changes.

Decodes hidden messages from stego images accurately.

Supports custom message sizes and filenames.

Modular design with separate source files for encoding, decoding, and file operations.

Strong emphasis on bit-level data manipulation.

🧩 Files Overview

main.c – Menu and user interaction

encode.c – Embeds secret messages into the image

decode.c – Extracts hidden data from the image

stego.h – Function prototypes and structure definitions

common.c – Utility functions for file and bit operations

⚙️ Compilation & Execution

To compile and run:

gcc main.c encode.c decode.c common.c -o stego
./stego

🧠 Example Workflow

Encode Mode

Enter option: Encode
Source image: original.bmp
Secret message file: secret.txt
Output image: hidden.bmp
Message encoded successfully!


Decode Mode

Enter option: Decode
Stego image: hidden.bmp
Output text file: output.txt
Secret message extracted successfully!

📚 Learning Outcomes

Learned bitwise manipulation and data embedding concepts

Enhanced understanding of binary file I/O and image formats

Strengthened C programming and modular code design skills

👩‍💻 Author

Likitha Pasam
GitHub: Likitha589
