# Quick Start Guide (Raspberry Pi)

0. Connect your Raspberry Pi to the internet and open it's console.

1. Install WiringPi:
    http://wiringpi.com/download-and-install/

2. Download and build Blynk:
    ```bash
    $ git clone https://github.com/blynkkk/blynk-library.git
    $ cd blynk-library/linux
    $ make clean all
    ```

3. : Pull files from https://github.com/rickyjericevich/EEE3096S/tree/master/Project/blynkfiles
     and place them in blynk-library/linux

4. : Build and run:
    ```bash
    $ make
    $ make run
    ```

We have also provided a build script, you can try just running (inside of the "linux" directory):

```bash
$ ./build.sh raspberry
```
