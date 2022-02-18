A wacky, hard to read analog clock intended to be 3D-printed and sold to students.

Eventually, this will be a complete accessory students can hang from their overalls. The clock will be displayed using a LED matrix, perhaps behind a cloudy plastic or acrylic piece. The ESP32 inside will be able to host it's own wireless AP and website for configuration, as well as synchronizing with other nearby clocks of the same type. 

Currently, a simple ESP32 board with a built-in OLED screen is used for testing. Unfortunately, I do not have permission to publish the display library for the OLED screen (which I got from a friend for a basic demo) and this project won't build without it, but eventually all the code and hardware designs will be free and open source. Note that while the files are technically C++, the project is nearly entirely in the C language. Conversion to C files is TBD. 

In this demo, the top of the clock face (ie. 12 o'clock) is indicated by the small line on the periphery. The speed of rotation will probably be slowed down for the final device (or even configurable).

https://user-images.githubusercontent.com/55636497/154678433-74f32c3b-af23-48da-a802-513ede81d8b4.mp4

