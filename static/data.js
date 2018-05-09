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
        constructor(id, status, position, map) {
            this.id = id;
            this.status = status;
            this.position = position;
            this.map = map;
            this.marker = new google.maps.Marker({
                position: { lat: this.position.lat, lng: this.position.lng },
                map: map,
                label: this.id.toString(),
                animation: google.maps.Animation.DROP /*,
                icon: 'static/auto_free.png'*/
            });
            this.updateInfoWindow();
        }
        updateInfoWindow() {
            var i = this.id, s = this.status;
            google.maps.event.clearListeners(this.marker, 'click');
            this.marker.addListener('click', function () {
                (new google.maps.InfoWindow({ content: "car: " + i + "<br>status: " + s })).open(this.map, this);
            });
        }
        ;
        update(newPosition, newStatus) {
            if (this.position != newPosition) {
                this.position = newPosition;
                this.marker.setPosition({ lat: this.position.lat, lng: this.position.lng });
            }
            if (this.status != newStatus) {
                this.status = newStatus;
                this.updateInfoWindow();
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
