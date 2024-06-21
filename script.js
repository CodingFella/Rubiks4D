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

let lastCall = 0;
const delay = 20;

let magnitude = 0.05;


async function startDemo() {
    const response = await fetch('out.wasm');
    const bytes = await response.arrayBuffer();
    const {instance} = await WebAssembly.instantiate(bytes);

    render(instance);

    document.getElementById("app").onclick = function(e) {
        // e = Mouse click event.
        let rect = e.target.getBoundingClientRect();
        let mouse_x = e.clientX - rect.left; //x position within the element.
        let mouse_y = e.clientY - rect.top;  //y position within the element.
        x = mouse_x;
        y = mouse_y;
        console.log("x : " + x + " ; y : " + y + ".");

        // const now = new Date().getTime();
        // if (now - lastCall < delay) {
        //     return;
        // }
        // lastCall = now;

        render(instance);
        x = -1;
        y = -1;
    }

    document.addEventListener("keydown", async(event) => {
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
        }
        console.log(current_input);
        render(instance);
    })
}

function render(instance) {
    const pixels = instance.exports.render(current_input, A, B, C, x, y);
    const buffer = instance.exports.memory.buffer;
    const imageData = new ImageData(new Uint8ClampedArray(buffer, pixels, app.width * app.height * 4), app.width);
    ctx.putImageData(imageData, 0, 0);
    current_input = 0;
}

startDemo();