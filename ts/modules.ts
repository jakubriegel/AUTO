namespace Module{
    abstract class Module{
        protected container: HTMLDivElement;

        constructor(panel: HTMLDivElement){
            // creating container for module
            this.container = document.createElement('div');
            this.container.className = 'module';
            panel.appendChild(this.container);
        }
    }

    export class RequestForm extends Module{
        private form: HTMLFormElement;

        private aInput: HTMLInputElement;
        private aSearchBox: google.maps.places.SearchBox;
        private aPosition: Data.Position;
        private aStatus: HTMLDivElement;

        private bInput: HTMLInputElement;
        private bSearchBox: google.maps.places.SearchBox;
        private bPosition: Data.Position;
        private bStatus: HTMLDivElement;

        private orderButton: HTMLInputElement;

        constructor(panel: HTMLDivElement){
            super(panel);
            // setting up the form
            this.form = <HTMLFormElement> document.createElement('form');
            this.form.id = 'order-form';

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
        }

        private initAutocomplete(): void {
            // set inputs as Google SearchBoxes and configure them
            this.aSearchBox = new google.maps.places.SearchBox(this.aInput);

            let callback = (searchBox: google.maps.places.SearchBox, position: Data.Position, status: HTMLDivElement) => {
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
                            status.textContent = JSON.parse(request.responseText).response;
                        }    
                    }
                    request.open("POST", "/auto/request/isAvailable", true); 
                    request.send(JSON.stringify(position));

                    return;
                }
                window.alert("Input correct place");
            }

            this.aSearchBox.addListener('places_changed', (form = this) => { 
                callback(form.aSearchBox, form.aPosition, form.aStatus); 
            });

            this.bSearchBox = new google.maps.places.SearchBox(this.bInput);
        
            this.bSearchBox.addListener('places_changed', (form = this) => { 
                callback(form.bSearchBox, form.bPosition, form.bStatus); 
            });
        }

        private sendOrder(form = this): void {
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

    export class Stats extends Module{
        private activeCars: number;
        private activeCarsDiv: HTMLDivElement;

        private freeCars: number;
        private freeCarsDiv: HTMLDivElement;

        private busyCars: number;
        private busyCarsDiv: HTMLDivElement;

        constructor(panel: HTMLDivElement){
            super(panel);

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

    export class CarList extends Module {
        private list: HTMLUListElement;
        private elements: Element[];

        constructor(panel: HTMLDivElement) {
            super(panel);

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