namespace Module{
    abstract class Module{
        protected container: HTMLDivElement;
        private header: HTMLDivElement;

        constructor(panel: HTMLDivElement, name: string, iconUrl: string, id: string){
            // create container for module
            this.container = document.createElement('div');
            this.container.className = 'module';
            this.container.id = id;
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
        // form
        private form: HTMLFormElement;
        private validation: FormValidation;
        // update activity of fields
        private checkValidity(val?: FormValidation): void {
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
            this.checkValidity({ a: false, b: false, val: undefined });
        }

        // info window
        private info: HTMLDialogElement;
        private infoContent: HTMLDivElement;

        constructor(panel: HTMLDivElement, private app: AUTO){
            super(panel, 'Order ride', 'order-icon.png', 'request-form');
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
            
            // set up info window
            this.info = <HTMLDialogElement> document.createElement('dialog');
            this.info.close();
            this.infoContent = <HTMLDivElement> document.createElement('section');
            this.info.appendChild(this.infoContent);
            let button = <HTMLButtonElement> document.createElement('button');
            button.onclick = () => { this.info.close(); this.infoContent.removeChild(this.infoContent.firstChild); };
            button.textContent = 'close';
            this.info.appendChild(button);

            // add form and info window to panel
            this.container.appendChild(this.form);
            this.container.appendChild(this.info);

            this.initAutocomplete();

            this.aPosition = new Data.Position(0,0);
            this.bPosition = new Data.Position(0,0);

            this.checkValidity();
        }

        private initAutocomplete(): void {
            // set inputs as Google SearchBoxes and configure them
            this.aSearchBox = new google.maps.places.SearchBox(this.aInput);

            let callback = (form: RequestForm, searchBox: google.maps.places.SearchBox, position: Data.Position, status: HTMLDivElement, validation: FormValidation) => {
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

                            form.checkValidity(validation);
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
            // show dialog to the user
            let placeholder = <HTMLImageElement> document.createElement('img');
            form.infoContent.appendChild(form.app.static.loading);
            form.info.showModal();
            let order = new Data.PositionsAB();
            order.A = form.aPosition;
            order.B = form.bPosition;
            // convert to JSON
            let orderJSON = JSON.stringify(order);
            let request = new XMLHttpRequest();
            request.onreadystatechange = function() {
                // tasks to be done after response is received
                if (this.readyState == 4 && this.status == 200) {
                    // process order data
                    let data = <Data.IOrderData> JSON.parse(request.responseText);
                    // prepare information
                    let content = <HTMLDivElement> document.createElement('div');
                    content.classList.add('order-info');

                    if(data.status == 1001) {
                        let img = <HTMLImageElement> document.createElement('img');
                        img.src = "/auto/static/car.png";
                        content.appendChild(img);
                        let heading = <HTMLHeadingElement> document.createElement('h3');
                        heading.textContent = "Ride details: ";
                        content.appendChild(heading);
                        let p = <HTMLParagraphElement> document.createElement('p');
                        p.classList.add('car');
                        p.textContent = "assinged car: " + data.job.id;
                        content.appendChild(p);
                        p = <HTMLParagraphElement> document.createElement('p');
                        p.classList.add('pre');
                        p.textContent = "arrive in: " + (data.job.preDuration / 60).toFixed(0) + 'min';
                        content.appendChild(p);
                        p = <HTMLParagraphElement> document.createElement('p');
                        p.classList.add('place');
                        p.textContent = "from: " + data.job.origin;
                        content.appendChild(p);
                        p = <HTMLParagraphElement> document.createElement('p');
                        p.classList.add('place');
                        p.textContent = "to: " + data.job.destination;
                        content.appendChild(p);
                        p = <HTMLParagraphElement> document.createElement('p');
                        p.classList.add('time');
                        p.textContent = "travel time: around " + (data.job.duration / 60).toFixed(0) + 'min';
                        content.appendChild(p);
                    }
                    else {
                        content.textContent = 'Your order was cancelled. Try again'
                    }

                    form.infoContent.removeChild(form.infoContent.firstChild);
                    //form.info.showModal();
                    form.infoContent.appendChild(content);

                }
            }
            request.open("POST", "/auto/request/route", true); 
            request.send(orderJSON);
        
            this.resetInputs();
        }
    }

    export class Overview extends Module{
        
        private activeCars: HTMLDivElement;
        private freeCars: HTMLDivElement;
        private busyCars: HTMLDivElement;
        private averagePre: HTMLDivElement;

        constructor(panel: HTMLDivElement){
            super(panel, 'Overview', 'overview-icon.png', 'overview');

            this.activeCars = document.createElement('div');
            this.activeCars.classList.add('number');
            this.container.appendChild(this.activeCars);
            let text: HTMLParagraphElement = document.createElement('p');
            text.textContent = 'active cars';
            this.container.appendChild(text);

            this.freeCars = document.createElement('div');
            this.freeCars.classList.add('number');
            this.container.appendChild(this.freeCars);
            text = document.createElement('p');
            text.textContent = 'free cars';
            this.container.appendChild(text);

            this.busyCars = document.createElement('div');
            this.busyCars.classList.add('number');
            this.container.appendChild(this.busyCars);
            text = document.createElement('p');
            text.textContent = 'busy cars';
            this.container.appendChild(text);

            this.averagePre = document.createElement('div');
            this.averagePre.classList.add('number');
            this.container.appendChild(this.averagePre);
            text = document.createElement('p');
            text.textContent = 'avg. waiting time';
            this.container.appendChild(text);

        }

        public update(active: number, free: number, busy: number, averagePre: number): void{
            this.activeCars.textContent = active.toString();
            this.freeCars.textContent = free.toString();
            this.busyCars.textContent = busy.toString();
            this.averagePre.textContent = (averagePre / 60).toFixed(0).toString() + 'min';
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
        private battery: HTMLDivElement;
        private destination: HTMLDivElement;
        private mileage: HTMLDivElement;
        private warnings: HTMLDivElement;

        constructor(car: Data.Car){
            this.car = car;

            this._li = <HTMLLIElement> document.createElement('li');
            this.id = <HTMLDivElement> document.createElement('div');
            this.id.classList.add('id');
            this.id.classList.add('element');
            this._li.appendChild(this.id);
            this.status = <HTMLDivElement> document.createElement('div');
            this.status.classList.add('status');
            this.status.classList.add('element');
            this._li.appendChild(this.status);
            this.battery = <HTMLDivElement> document.createElement('div');
            this.battery.classList.add('battery');
            this.battery.classList.add('element');
            this._li.appendChild(this.battery);
            this.destination = <HTMLDivElement> document.createElement('div');
            this.destination.classList.add('destination');
            this.destination.classList.add('element');
            this._li.appendChild(this.destination);
            this.mileage = <HTMLDivElement> document.createElement('div');
            this.mileage.classList.add('mileage');
            this.mileage.classList.add('element');
            this._li.appendChild(this.mileage);
            this.warnings = <HTMLDivElement> document.createElement('div');
            this.warnings.classList.add('warnings');
            this.warnings.classList.add('element');
            this._li.appendChild(this.warnings);
            
            this.update();
        }

        public update(): void {
            this.id.textContent = this.car.getId().toString();
            this.status.textContent = this.car.getStatus().toString();
            this.battery.textContent = (this.car.getBattery() / 10000).toFixed(0).toString() + '%';
            this.destination.textContent = this.car.getDestination();
            this.mileage.textContent = (this.car.getMileage() / 1000).toFixed(0).toString() + 'km';
            let w = this.car.getWarnigsStr();
            this.warnings.textContent =  w == '' ? '-' : w;
        }
    }

    export class Stats extends Module {
        private list: HTMLUListElement;
        private elements: Element[];

        constructor(panel: HTMLDivElement) {
            super(panel, 'Stats', 'stats-icon.png', 'stats');

            // create header
            let header = <HTMLHeadingElement> document.createElement('h3');
            let temp: HTMLDivElement = document.createElement('div');
            temp.classList.add('id');
            temp.classList.add('element');
            temp.classList.add('list-header');
            temp.textContent = 'id';
            this.container.appendChild(temp);
            temp = document.createElement('div');
            temp.classList.add('status');
            temp.classList.add('element');
            temp.classList.add('list-header');
            temp.textContent = 'status';
            this.container.appendChild(temp);
            temp = document.createElement('div');
            temp.classList.add('battery');
            temp.classList.add('element');
            temp.classList.add('list-header');
            temp.textContent = 'batt.';
            this.container.appendChild(temp);
            temp = document.createElement('div');
            temp.classList.add('destination');
            temp.classList.add('element');
            temp.classList.add('list-header');
            temp.textContent = 'destination';
            this.container.appendChild(temp);
            temp = document.createElement('div');
            temp.classList.add('mileage');
            temp.classList.add('element');
            temp.classList.add('list-header');
            temp.textContent = 'mil.';
            this.container.appendChild(temp);
            temp = document.createElement('div');
            temp.classList.add('warnings');
            temp.classList.add('element');
            temp.classList.add('list-header');
            temp.textContent = 'warnings';
            this.container.appendChild(temp);

            // create list
            this.list = <HTMLUListElement> document.createElement('ul');
            this.container.appendChild(this.list);

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