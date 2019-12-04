# Tom and Jerry

A game based off Tom and Jerry for the CAB202 unit at QUT. In this assignment, I was tasked with bringing to life the classic characters of Tom and Jerry in a terminal-based game. Tom is a cat with the simple agenda of catching the mouse, Jerry, running rampant in his house. Tom runs around the room Jerry is in, dropping mousetraps periodically, and constantly changing his direction to move toward Jerry. Jerry just wants cheese and is faster than Tom, but after he has consumed enough cheese, and taken the chase to the second room of the day, he likes to have a bit of fun by introducing some weapons of his own to direct at Tom.

## Issues with the code

If I recall correctly the only issue I remember is that diagonal collisions didn't work well (I rectified this in the next assessment).

## Installation

Clone it and go into it

`cd ZDK`
`make`
`cd ..`
`gcc TomJerryGame.c -o game -std=gnu99 -Wall -Werror -IZDK -LZDK -lzdk -lncurses`

## Usage

```bash
./game examples/room00.txt examples/room01.txt examples/room02.txt examples/room03.txt examples/room04.txt examples/room05.txt examples/room06.txt examples/room07.txt examples/room08.txt examples/room09.txt
```

## Contributing

No contributions as this was a project for a university class that is now finished.

## License
[MIT](https://choosealicense.com/licenses/mit/)