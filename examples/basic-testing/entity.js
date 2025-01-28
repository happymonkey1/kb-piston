
const foo = 'foo';

function onInit() {
    console.log('Running entity.js onInit()!');
}

let updateCounter = 0;

function onUpdate() {
    if (updateCounter % 1000 === 0) {
        console.log(`Hello from entity.js! deltaTime=${Time.deltaTime}`);
        updateCounter = 0;
    }

    updateCounter++;

    //console.log('test')

    return foo;
};
