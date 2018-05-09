var Module;
(function (Module_1) {
    class Module {
        constructor(panel) {
            // creating container for module
            this.container = document.createElement('div');
            this.container.className = 'module';
            panel.appendChild(this.container);
        }
    }
    class RequestForm extends Module {
        constructor(panel) {
            super(panel);
            // setting up the form
            this.form = document.createElement('form');
            this.form.id = 'order-form';
            this.aInput = document.createElement('input');
            this.aInput.type = 'text';
            this.aInput.placeholder = 'Enter origin';
            this.form.appendChild(this.aInput);
            this.bInput = document.createElement('input');
            this.bInput.type = 'text';
            this.bInput.placeholder = 'Enter destination';
            this.form.appendChild(this.bInput);
            this.orderButton = document.createElement('input');
            this.orderButton.type = 'button';
            this.orderButton.value = 'Request';
            this.orderButton.onclick = () => { this.sendOrder(this); };
            this.form.appendChild(this.orderButton);
            // add form to panel
            this.container.appendChild(this.form);
            this.initAutocomplete();
            this.aPosition = new Data.Position(0, 0);
            this.bPosition = new Data.Position(0, 0);
        }
        initAutocomplete() {
            // set inputs as Google SearchBoxes and configure them
            this.aSearchBox = new google.maps.places.SearchBox(this.aInput);
            this.aSearchBox.addListener('places_changed', (form = this) => {
                let places = form.aSearchBox.getPlaces();
                if (places.length == 0)
                    return;
                if (places.length == 1) {
                    form.aPosition = new Data.Position(places[0].geometry.location.lat(), places[0].geometry.location.lng());
                    console.log(form.aPosition);
                    return;
                }
                window.alert("Input correct place");
            });
            this.bSearchBox = new google.maps.places.SearchBox(this.bInput);
            this.bSearchBox.addListener('places_changed', (form = this) => {
                let places = form.bSearchBox.getPlaces();
                if (places.length == 0)
                    return;
                if (places.length == 1) {
                    form.bPosition = new Data.Position(places[0].geometry.location.lat(), places[0].geometry.location.lng());
                    console.log(form.bPosition);
                    return;
                }
                window.alert("Input correct place");
            });
        }
        sendOrder(form = this) {
            let order = new Data.PositionsAB();
            order.A = form.aPosition;
            order.B = form.bPosition;
            // convert to JSON
            let orderJSON = JSON.stringify(order);
            let request = new XMLHttpRequest();
            request.open("POST", "/auto/request/route", false);
            request.send(orderJSON);
            alert("Your car id is " + request.responseText);
        }
    }
    Module_1.RequestForm = RequestForm;
    class Stats extends Module {
        constructor(panel) {
            super(panel);
            this.activeCars = -1;
            this.activeCarsDiv = document.createElement('div');
            this.container.appendChild(this.activeCarsDiv);
            let text = document.createElement('p');
            text.textContent = 'active cars';
            this.container.appendChild(text);
            this.freeCars = -1;
            this.freeCarsDiv = document.createElement('div');
            this.container.appendChild(this.freeCarsDiv);
            text = document.createElement('p');
            text.textContent = 'free cars';
            this.container.appendChild(text);
            this.busyCars = -1;
            this.busyCarsDiv = document.createElement('div');
            this.container.appendChild(this.busyCarsDiv);
            text = document.createElement('p');
            text.textContent = 'busy cars';
            this.container.appendChild(text);
            // this.update();
        }
        update(stats) {
            this.activeCars = stats['active'];
            this.freeCars = stats['free'];
            this.busyCars = stats['busy'];
            this.activeCarsDiv.textContent = this.activeCars.toString();
            this.freeCarsDiv.textContent = this.freeCars.toString();
            this.busyCarsDiv.textContent = this.busyCars.toString();
        }
    }
    Module_1.Stats = Stats;
    // element of CarList
    class Element {
        get li() { return this._li; }
        getId() { return this.car.getId(); }
        constructor(car) {
            this.car = car;
            this._li = document.createElement('li');
            this.id = document.createElement('div');
            this.id.className = 'id';
            this.status = document.createElement('div');
            this.status.className = 'status';
            this._li.appendChild(this.id);
            this._li.appendChild(this.status);
            this.update();
        }
        update() {
            this.id.textContent = this.car.getId().toString();
            this.status.textContent = this.car.getStatus().toString();
        }
    }
    class CarList extends Module {
        constructor(panel) {
            super(panel);
            // create container
            let box = document.createElement('div');
            box.id = 'car-list-container';
            // create header
            let header = document.createElement('h3');
            let temp = document.createElement('div');
            temp.className = 'id header';
            temp.textContent = 'id';
            box.appendChild(temp);
            temp = document.createElement('div');
            temp.className = 'status header';
            temp.textContent = 'status';
            box.appendChild(temp);
            // create list
            this.list = document.createElement('ul');
            box.appendChild(this.list);
            this.container.appendChild(box);
            // initialize elements
            this.elements = [];
        }
        update(cars) {
            for (let car of cars) {
                (() => {
                    for (let element of this.elements)
                        if (element.getId() == car.getId()) {
                            element.update();
                            return;
                        }
                    let newElement = new Element(car);
                    this.list.appendChild(newElement.li);
                    newElement.update();
                    this.elements.push(newElement);
                })();
            }
        }
    }
    Module_1.CarList = CarList;
})(Module || (Module = {}));
