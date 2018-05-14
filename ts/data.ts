
namespace Data{
    export class Position {
        constructor(public lat: number, public lng: number){};
    }

    export class PositionsAB {
        A: Position;
        B: Position;
    }

    export class Car {
        private static staticData: Static;
        public static setStatic(data: Static) { Car.staticData = data; }

        private marker: google.maps.Marker;
        private updateMarker() {
            var i = this.id, s = this.status;
            google.maps.event.clearListeners(this.marker, 'click');
            this.marker.addListener('click', function() {
                (new google.maps.InfoWindow({ content: "car: " + i + "<br>status: " + s })).open(this.map, this);
            });

            switch (this.status) {
                case 101:
                    this.marker.setIcon(Car.staticData.autoFree);
                    break;
                case 102:
                    this.marker.setIcon(Car.staticData.autoTaken);
                    break;
                case 103:
                    this.marker.setIcon(Car.staticData.autoBusy);
                    break;
            }

        }

        constructor(private id: number, private status: number, private position: Position, private map: google.maps.Map, private staticData: Static) {            
            this.marker = new google.maps.Marker({ 
                position: {lat: this.position.lat, lng: this.position.lng}, 
                map: map, 
                //label : this.id.toString(),
                animation : google.maps.Animation.DROP, 
                icon: Car.staticData.autoFree
            });
            this.updateMarker();
        };

        public update(newPosition: Position, newStatus: number){
            if(this.position != newPosition){
                this.position = newPosition;
                this.marker.setPosition({lat: this.position.lat, lng: this.position.lng});
            }
            if(this.status != newStatus){
                this.status = newStatus;
                this.updateMarker();
            }
        }

        public getId(){ return this.id; };
        public getStatus(){ return this.status; };
        public getPosition(){ return this.position };
    }

    export interface ICarData{
        stats: Map<string, number>;
        cars: Array<any>;
    }
}