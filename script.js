let app = document.getElementById("app");
app.width = 800;
app.height = 600;
let ctx = app.getContext("2d");

let current_input = 0;

let A = 0;
let B = 0;
let C = 0;

let x = 0;
let y = 0;

let dt = 0;

let current_cube = 0;
let current_face = 0;

let to_rotate = 0;

let lastCall = 0;


let delay = 0;

let angle_percent = 0;
let angle_jump = 15;
let type = -1;

let magnitude = 0.05;

let rotating = false;


async function startDemo() {
    const response = await fetch('out.wasm');
    const bytes = await response.arrayBuffer();
    const {instance} = await WebAssembly.instantiate(bytes);

    render(instance);

    // document.getElementById("app").onclick = function(e) {
    //     // e = Mouse click event.
    //     let rect = e.target.getBoundingClientRect();
    //     let mouse_x = e.clientX - rect.left; //x position within the element.
    //     let mouse_y = e.clientY - rect.top;  //y position within the element.
    //     x = mouse_x;
    //     y = mouse_y;
    //     console.log("x : " + x + " ; y : " + y + ".");
    //
    //     // const now = new Date().getTime();
    //     // if (now - lastCall < delay) {
    //     //     return;
    //     // }
    //     // lastCall = now;
    //
    //     render(instance);
    //     x = -1;
    //     y = -1;
    // }



    document.addEventListener("keydown", async(event) => {
        if(rotating) {
            return;
        }
        if(event.key === 'w') {
            current_input = 1;
            A += magnitude;
        } else if(event.key === 'a') {
            current_input = 2;
            B -= magnitude;
        } else if(event.key === 's') {
            current_input = 3;
            A -= magnitude;
        } else if(event.key === 'd') {
            current_input = 4;
            B += magnitude;
        } else if(event.key === 'j') {
            current_input = 5;
            C += magnitude;
        } else if(event.key === 'k') {
            current_input = 6;
            C -= magnitude;
        } else if(event.key === '.') {
            current_cube += 1;
            if(current_cube > 26) {
                current_cube = 26;
            }
        } else if(event.key === ',') {
            current_cube -= 1;
            if(current_cube < 0) {
                current_cube = 0;
            }
        } else if (event.key === '1') {
            current_face = 0;
            current_cube = 0;
        } else if (event.key === '2') {
            current_face = 1;
            current_cube = 0;
        } else if (event.key === '3') {
            current_face = 2;
            current_cube = 0;
        } else if (event.key === '4') {
            current_face = 3;
            current_cube = 0;
        } else if (event.key === '5') {
            current_face = 4;
            current_cube = 0;
        } else if (event.key === '6') {
            current_face = 5;
            current_cube = 0;
        } else if (event.key === '7') {
            current_face = 6;
            current_cube = 0;
        }
        // else if (event.key === '8') {
        //     current_face = 7;
        //     current_cube = 0;
        // }
        else if (event.key === 'r') {
            if(rotating) {
                return;
            }

            rotating = true;
            to_rotate = 0;

            for(let i = 1; i <= 100; i+= angle_jump) {
                angle_percent = i;
                render(instance);
                await new Promise(resolve => setTimeout(resolve));
            }

            to_rotate = 1;

            rotating = false;


        } else if (event.key === '/') {
            current_cube += 9;
            if(current_cube > 26) {
                current_cube = 26;
            }
        } else if (event.key === 'o') {
            angle_percent -= 1;
            if(angle_percent < 0) {
                angle_percent = 0;
            }
        } else if (event.key === 'p') {
            angle_percent += 1;
            if(angle_percent > 100) {
                angle_percent = 100;
            }
        }
        // console.log(current_input);
        render(instance);
    })
}

function render(instance) {
    const pixels = instance.exports.render(dt, current_input, A, B, C, x, y, current_face * 27 + current_cube, to_rotate, angle_percent, type);
    const buffer = instance.exports.memory.buffer;
    const imageData = new ImageData(new Uint8ClampedArray(buffer, pixels, app.width * app.height * 4), app.width);
    ctx.putImageData(imageData, 0, 0);
    current_input = 0;
    dt += 1;
    to_rotate = 0;
    angle_percent = 0;
    type = -1;
}

startDemo();