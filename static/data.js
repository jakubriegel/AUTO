var Data;
(function (Data) {
    class Position {
        constructor(lat, lng) {
            this.lat = lat;
            this.lng = lng;
        }
        ;
    }
    Data.Position = Position;
    class PositionsAB {
    }
    Data.PositionsAB = PositionsAB;
    class Car {
        constructor(id, status, position, map, staticData) {
            this.id = id;
            this.status = status;
            this.position = position;
            this.map = map;
            this.staticData = staticData;
            this.marker = new google.maps.Marker({
                position: { lat: this.position.lat, lng: this.position.lng },
                map: map,
                //label : this.id.toString(),
                animation: google.maps.Animation.DROP,
                icon: Car.staticData.autoFree
            });
            this.updateMarker();
        }
        static setStatic(data) { Car.staticData = data; }
        updateMarker() {
            var i = this.id, s = this.status;
            google.maps.event.clearListeners(this.marker, 'click');
            this.marker.addListener('click', function () {
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
        ;
        update(newPosition, newStatus) {
            if (this.position != newPosition) {
                this.position = newPosition;
                this.marker.setPosition({ lat: this.position.lat, lng: this.position.lng });
            }
            if (this.status != newStatus) {
                this.status = newStatus;
                this.updateMarker();
            }
        }
        getId() { return this.id; }
        ;
        getStatus() { return this.status; }
        ;
        getPosition() { return this.position; }
        ;
    }
    Data.Car = Car;
})(Data || (Data = {}));
