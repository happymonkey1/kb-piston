
const foo = 'foo';

function onInit() {
    console.log('Running entity.js onInit()!');
}

let updateCounter = 0;

function onUpdate() {
    if (updateCounter % 1000 === 0) {
        console.log('Hello from entity.js!');
    }

    updateCounter++;

    //console.log('test')

    return foo;
};
