var Module;
(function (Module_1) {
    class Module {
        constructor(panel, name, iconUrl) {
            // create container for module
            this.container = document.createElement('div');
            this.container.className = 'module';
            panel.appendChild(this.container);
            // create header
            this.header = document.createElement('div');
            this.header.className = 'header';
            let icon = document.createElement('div');
            icon.className = 'icon';
            icon.style.backgroundImage = 'url("auto/static/' + iconUrl + '")';
            this.header.appendChild(icon);
            let title = document.createElement('h3');
            title.textContent = name;
            this.header.appendChild(title);
            this.container.appendChild(this.header);
        }
    }
    class RequestForm extends Module {
        constructor(panel) {
            super(panel, 'Order ride', 'order-icon.png');
            // setting up the form
            this.form = document.createElement('form');
            this.form.id = 'order-form';
            this.validation = { a: false, b: false };
            this.aInput = document.createElement('input');
            this.aInput.type = 'text';
            this.aInput.placeholder = 'Enter origin';
            this.form.appendChild(this.aInput);
            this.aStatus = document.createElement('div');
            this.aStatus.className = 'status-icon';
            this.form.appendChild(this.aStatus);
            this.bInput = document.createElement('input');
            this.bInput.type = 'text';
            this.bInput.placeholder = 'Enter destination';
            this.form.appendChild(this.bInput);
            this.bStatus = document.createElement('div');
            this.bStatus.className = 'status-icon';
            this.form.appendChild(this.bStatus);
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
            this.checkValidation();
        }
        // update activity of fields
        checkValidation(val) {
            // check if new values of validation were passed
            if (val) {
                if (val.val == undefined)
                    this.validation = val;
                else {
                    if (val.a)
                        this.validation.a = val.val;
                    else
                        this.validation.b = val.val;
                }
            }
            if (this.validation.a) {
                this.bInput.disabled = false;
                if (this.validation.b)
                    this.orderButton.disabled = false;
            }
            else {
                this.bInput.disabled = true;
                this.orderButton.disabled = true;
            }
        }
        // clear inputs values and set their validation to false
        resetInputs() {
            this.aInput.value = "";
            this.aStatus.className = "status-icon";
            this.bInput.value = "";
            this.bStatus.className = "status-icon";
            this.checkValidation({ a: false, b: false, val: undefined });
        }
        initAutocomplete() {
            // set inputs as Google SearchBoxes and configure them
            this.aSearchBox = new google.maps.places.SearchBox(this.aInput);
            let callback = (form, searchBox, position, status, validation) => {
                let places = searchBox.getPlaces();
                if (places.length == 0)
                    return;
                if (places.length == 1) {
                    position.lat = places[0].geometry.location.lat();
                    position.lng = places[0].geometry.location.lng();
                    // ask for car avaibality and print response
                    let request = new XMLHttpRequest();
                    request.onreadystatechange = function () {
                        // tasks to be done after response is received
                        if (this.readyState == 4 && this.status == 200) {
                            switch (JSON.parse(request.responseText).response) {
                                case 201:
                                    status.className = 'status-icon available';
                                    validation.val = true;
                                    break;
                                case 202:
                                case 203:
                                    status.className = 'status-icon not-available';
                                    validation.val = false;
                                    break;
                            }
                            form.checkValidation(validation);
                        }
                    };
                    request.open("POST", "/auto/request/isAvailable", true);
                    request.send(JSON.stringify(position));
                    return;
                }
                window.alert("Input correct place");
            };
            this.aSearchBox.addListener('places_changed', (form = this) => {
                callback(form, form.aSearchBox, form.aPosition, form.aStatus, { a: true, b: false });
            });
            this.bSearchBox = new google.maps.places.SearchBox(this.bInput);
            this.bSearchBox.addListener('places_changed', (form = this) => {
                callback(form, form.bSearchBox, form.bPosition, form.bStatus, { a: false, b: true });
            });
        }
        sendOrder(form = this) {
            let order = new Data.PositionsAB();
            order.A = form.aPosition;
            order.B = form.bPosition;
            // convert to JSON
            let orderJSON = JSON.stringify(order);
            let request = new XMLHttpRequest();
            request.onreadystatechange = function () {
                // tasks to be done after response is received
                if (this.readyState == 4 && this.status == 200)
                    alert("Your car id is " + request.responseText);
            };
            request.open("POST", "/auto/request/route", true);
            request.send(orderJSON);
            this.resetInputs();
        }
    }
    Module_1.RequestForm = RequestForm;
    class Overview extends Module {
        constructor(panel) {
            super(panel, 'Overview', 'overview-icon.png');
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
    Module_1.Overview = Overview;
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
    class Stats extends Module {
        constructor(panel) {
            super(panel, 'Stats', 'stats-icon.png');
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
    Module_1.Stats = Stats;
})(Module || (Module = {}));
