const beer = {
    name: 'Lager',
    abv: 4.2,
    ibu: 15,
    price: 5.00,
};

function onInit() {
    console.log('Running serialization.js onInit()!');
    const beerString = JSON.stringify(beer);
    console.log(`beer = ${beerString}`);
    
    return beerString;
}

let updateCounter = 0;

function onUpdate() {
    if (updateCounter % 500 === 0) {
        //console.log('Hello from serialization.js!');
        updateCounter = 0;
    }
    
    updateCounter++;
    
    //console.log('test2')
}
