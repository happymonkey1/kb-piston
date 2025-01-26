
const foo = 'foo';

function onInit() {
    console.log('Running entity.js onInit()!');
}

function onUpdate() {
    console.log('Hello from entity.js!');

    return foo;
};
