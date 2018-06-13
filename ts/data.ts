
namespace Data{
    export class Position {
        constructor(public lat: number, public lng: number){};
    }

    export class PositionsAB {
        A: Position;
        B: Position;
    }

    export abstract class Item {
        protected static staticData: Static;
        public static setStatic(data: Static) { Item.staticData = data; }

        protected root: Item;

        protected marker: google.maps.Marker;
        protected updateMarker(infoContent: string) {
            // remove existing listeners
            google.maps.event.clearListeners(this.marker, 'click');
            // add event listener for 'click' to display info
            this.marker.addListener('click', function() {
                (new google.maps.InfoWindow({ content: infoContent })).open(this.map, this);
            });
        }

        protected constructor (protected id: number, protected position: Position, protected map: google.maps.Map, _icon: google.maps.Icon) {
            this.marker = new google.maps.Marker({ 
                position: {lat: this.position.lat, lng: this.position.lng}, 
                map: map, 
                animation : google.maps.Animation.DROP, 
                icon: _icon
            });
        }

        // getters to private members
        public getId(){ return this.id; };
        public getPosition(){ return this.position };
    }

    export class Car extends Item{
        constructor(_id: number, private status: number, _position: Position, private battery: number, private mileage: number, private destination: string, private warnings: number[], _map: google.maps.Map) {            
            super(_id, _position, _map,Car.staticData.autoFree);
            
            this.updateMarker();
        };

        // override
        protected updateMarker() {
            super.updateMarker(
                            "car: " + this.id + 
                            "<br>status: " + this.status + 
                            "<br>battery: " + Math.round(this.battery / 10000) + "%" +
                            "<br>mileage: " + (this.mileage / 1000).toFixed(2) + "km" +
                            "<br>destination: " + this.destination +
                            "<br>warnings: " + this.getWarnigsStr());
            // switch marker icon
            switch (this.status) {
                case 101: // FREE
                case 104: // IN_BASE
                    this.marker.setIcon(Car.staticData.autoFree);
                    break;
                case 102: // JOB
                    this.marker.setIcon(Car.staticData.autoTaken);
                    break;
                case 103: // PRE_JOB
                case 105: // TO_BASE
                    this.marker.setIcon(Car.staticData.autoBusy);
                    break;
            }
            
            // hide marker if the car status is IN_BASE/PORT or CHARGING
            if(this.status == 104 || this.status == 106 || this.status == 107) this.marker.setVisible(false);
            else this.marker.setVisible(true);

        }

        public update(newPosition: Position, newStatus: number, newBattery: number, newMileage: number, newDest: string, newWarns: number[]){
            if(this.position != newPosition){
                this.position = newPosition;
                this.marker.setPosition({lat: this.position.lat, lng: this.position.lng});
            }
            
            this.status = newStatus;
            this.battery = newBattery;
            this.mileage = newMileage;
            this.destination = newDest;
            this.warnings = newWarns;
            this.updateMarker();
        }

        // getters to private members
        public getStatus(): number { return this.status; };
        public getBattery(): number { return this.battery; }
        public getMileage(): number { return this.mileage; }
        public getDestination(): string { return this.destination; }
        public getWarnigsStr(): string {
            let w = '';
            for(let i of this.warnings) w += i + ' ';
            return w;
        }
    }

    export class Base extends Item{
        constructor(
            _id: number, _position: Position, _map: google.maps.Map, 
            private name: string, private slots: number, private free: number, 
            private docked: number[], private reserved: number[], icon: google.maps.Icon = Car.staticData.base
            ) {            
            super(_id, _position, _map, icon);

            this.updateMarker();
        };

        // override
        protected updateMarker() {
            super.updateMarker(
                'base: ' + this.name + 
                '<br>id: ' + this.id + 
                '<br>free slots: ' + this.free + '/' + this.slots + 
                '<br>docked cars: ' + this.docked.toString() + 
                '<br>reserved cars: ' + this.reserved.toString()
            );
        }

        public update(newFree: number, newDocked: number[], newReserved: number[]){
            this.free = newFree;
            this.docked = newDocked;
            this.reserved = newReserved;
            this.updateMarker();
        }
    }

    export class Port extends Base {
        constructor(
            _id: number, _position: Position, _map: google.maps.Map, 
            name: string, slots: number, free: number, 
            docked: number[], reserved: number[]
            ) {            
            super(_id, _position, _map, name, slots,free, docked, reserved, Car.staticData.port);
        };

    }

    export interface IUpdateData {
        stats: {
            active: number;
            free: number;
            busy: number;
            preTime: number;
            rides: number;
        };
        cars: Array<any>;
        bases: Array<any>;
        port: any;
    }

    export interface IOrderData {
        status: number;
        job: {
            id: number;
            origin: string;
            destination: string;
            duration: number;
            preDuration: number;
        }
    }
}