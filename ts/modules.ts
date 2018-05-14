namespace Module{
    abstract class Module{
        protected container: HTMLDivElement;
        private header: HTMLDivElement;

        constructor(panel: HTMLDivElement, name: string, iconUrl: string){
            // create container for module
            this.container = document.createElement('div');
            this.container.className = 'module';
            panel.appendChild(this.container);

            // create header
            this.header = <HTMLDivElement> document.createElement('div');
            this.header.className = 'header';
            let icon = document.createElement('div');
            icon.className = 'icon';
            icon.style.backgroundImage = 'url("auto/static/' + iconUrl + '")';
            this.header.appendChild(icon);
            let title = <HTMLHeadingElement> document.createElement('h3');
            title.textContent = name;
            this.header.appendChild(title);

            this.container.appendChild(this.header);

        }
    }

    interface FormValidation{
        a: boolean;
        b: boolean;
        val?: boolean;
    }

    export class RequestForm extends Module{
        private form: HTMLFormElement;
        private validation: FormValidation;
        // update activity of fields
        private checkValidation(val?: FormValidation): void {
            // check if new values of validation were passed
            if(val){
                if(val.val == undefined) this.validation = val;
                else{
                    if(val.a) this.validation.a = val.val;
                    else this.validation.b = val.val;
                }
            }

            if(this.validation.a){
                this.bInput.disabled = false;
                if(this.validation.b) this.orderButton.disabled = false;
            }
            else{
                this.bInput.disabled = true;
                this.orderButton.disabled = true;
            }
        }

        private aInput: HTMLInputElement;
        private aSearchBox: google.maps.places.SearchBox;
        private aPosition: Data.Position;
        private aStatus: HTMLDivElement;

        private bInput: HTMLInputElement;
        private bSearchBox: google.maps.places.SearchBox;
        private bPosition: Data.Position;
        private bStatus: HTMLDivElement;

        private orderButton: HTMLInputElement;

        // clear inputs values and set their validation to false
        private resetInputs(): void {
            this.aInput.value = "";
            this.aStatus.className = "status-icon";
            this.bInput.value = "";
            this.bStatus.className = "status-icon";
            this.checkValidation({ a: false, b: false, val: undefined });
        }

        constructor(panel: HTMLDivElement){
            super(panel, 'Order ride', 'order-icon.png');
            // setting up the form
            this.form = <HTMLFormElement> document.createElement('form');
            this.form.id = 'order-form';
            this.validation = { a: false, b: false };

            this.aInput = <HTMLInputElement> document.createElement('input');
            this.aInput.type = 'text';
            this.aInput.placeholder = 'Enter origin';
            this.form.appendChild(this.aInput);

            this.aStatus = <HTMLDivElement> document.createElement('div');
            this.aStatus.className = 'status-icon';
            this.form.appendChild(this.aStatus);
            
            this.bInput = <HTMLInputElement> document.createElement('input');
            this.bInput.type = 'text';
            this.bInput.placeholder = 'Enter destination';
            this.form.appendChild(this.bInput);

            this.bStatus = <HTMLDivElement> document.createElement('div');
            this.bStatus.className = 'status-icon';
            this.form.appendChild(this.bStatus);
            
            this.orderButton = <HTMLInputElement> document.createElement('input');
            this.orderButton.type = 'button';
            this.orderButton.value = 'Request';
            this.orderButton.onclick = () => { this.sendOrder(this); };
            this.form.appendChild(this.orderButton);
            
            // add form to panel
            this.container.appendChild(this.form);

            this.initAutocomplete();

            this.aPosition = new Data.Position(0,0);
            this.bPosition = new Data.Position(0,0);

            this.checkValidation();
        }

        private initAutocomplete(): void {
            // set inputs as Google SearchBoxes and configure them
            this.aSearchBox = new google.maps.places.SearchBox(this.aInput);

            let callback = (form: RequestForm, searchBox: google.maps.places.SearchBox, position: Data.Position, status: HTMLDivElement, validation) => {
                let places = searchBox.getPlaces();
                if(places.length == 0) return;
                if(places.length == 1) {
                    position.lat = places[0].geometry.location.lat();
                    position.lng = places[0].geometry.location.lng()

                    // ask for car avaibality and print response
                    let request = new XMLHttpRequest();
                    request.onreadystatechange = function() {
                        // tasks to be done after response is received
                        if (this.readyState == 4 && this.status == 200){
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
                    }
                    request.open("POST", "/auto/request/isAvailable", true); 
                    request.send(JSON.stringify(position));

                    return;
                }
                window.alert("Input correct place");
            }

            this.aSearchBox.addListener('places_changed', (form = this) => { 
                callback(form, form.aSearchBox, form.aPosition, form.aStatus, { a: true, b: false }); 
            });

            this.bSearchBox = new google.maps.places.SearchBox(this.bInput);
        
            this.bSearchBox.addListener('places_changed', (form = this) => { 
                callback(form, form.bSearchBox, form.bPosition, form.bStatus, { a: false, b: true }); 
            });
        }

        private sendOrder(form = this): void {
            let order = new Data.PositionsAB();
            order.A = form.aPosition;
            order.B = form.bPosition;
            // convert to JSON
            let orderJSON = JSON.stringify(order);
            let request = new XMLHttpRequest();
            request.onreadystatechange = function() {
                // tasks to be done after response is received
                if (this.readyState == 4 && this.status == 200) alert("Your car id is " + request.responseText);    
            }
            request.open("POST", "/auto/request/route", true); 
            request.send(orderJSON);
        
            this.resetInputs();
        }
    }

    export class Overview extends Module{
        private activeCars: number;
        private activeCarsDiv: HTMLDivElement;

        private freeCars: number;
        private freeCarsDiv: HTMLDivElement;

        private busyCars: number;
        private busyCarsDiv: HTMLDivElement;

        constructor(panel: HTMLDivElement){
            super(panel, 'Overview', 'overview-icon.png');

            this.activeCars = -1;
            this.activeCarsDiv = document.createElement('div');
            this.container.appendChild(this.activeCarsDiv);
            let text: HTMLParagraphElement = document.createElement('p');
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

        public update(stats: Map<string, number>): void{
            this.activeCars = stats['active'];
            this.freeCars = stats['free'];
            this.busyCars = stats['busy'];

            this.activeCarsDiv.textContent = this.activeCars.toString();
            this.freeCarsDiv.textContent = this.freeCars.toString();
            this.busyCarsDiv.textContent = this.busyCars.toString();
        }

    }

    // element of CarList
    class Element {
        private _li: HTMLLIElement;
        get li(): HTMLLIElement { return this._li; }
        private car: Data.Car;
        public getId(): number { return this.car.getId(); }

        private id: HTMLDivElement;
        private status: HTMLDivElement;

        constructor(car: Data.Car){
            this.car = car;

            this._li = <HTMLLIElement> document.createElement('li');
            this.id = <HTMLDivElement> document.createElement('div');
            this.id.className = 'id';
            this.status = <HTMLDivElement> document.createElement('div');
            this.status.className = 'status';
            this._li.appendChild(this.id);
            this._li.appendChild(this.status);

            this.update();
        }

        public update(): void {
            this.id.textContent = this.car.getId().toString();
            this.status.textContent = this.car.getStatus().toString();
        }
    }

    export class Stats extends Module {
        private list: HTMLUListElement;
        private elements: Element[];

        constructor(panel: HTMLDivElement) {
            super(panel, 'Stats', 'stats-icon.png');

            // create container
            let box = <HTMLDivElement> document.createElement('div');
            box.id = 'car-list-container';
            
            // create header
            let header = <HTMLHeadingElement> document.createElement('h3');
            let temp: HTMLDivElement = document.createElement('div');
            temp.className = 'id header';
            temp.textContent = 'id';
            box.appendChild(temp);
            temp = document.createElement('div');
            temp.className = 'status header';
            temp.textContent = 'status';
            box.appendChild(temp);

            // create list
            this.list = <HTMLUListElement> document.createElement('ul');
            box.appendChild(this.list);
            this.container.appendChild(box);

            // initialize elements
            this.elements = [];
        }

        public update(cars: Data.Car[]): void {
            for(let car of cars) {
                (() => {
                    for(let element of this.elements) if(element.getId() == car.getId()) {
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
}