function sleep(milliseconds) {
    const date = Date.now();
    let currentDate = null;
    do {
      currentDate = Date.now();
    } while (currentDate - date < milliseconds);
    return;
}

async function aWhile() {
    console.log("Async call started");
    sleep(3000);
    console.log("Async call finished");
}


async function layer() {
    console.log("Layer call started");
    const test = sleep(2000);
    console.log("Before await");
    await test;
    console.log("After await");
    console.log("Layer call ended");
}

setTimeout(aWhile, 0);
layer();
console.log("Exited layer function");
let i = 0;
while (i < 5) {
    i ++;
    sleep(1000);
    console.log(`Blocking on the main thread for ${5-i} secs`);
}