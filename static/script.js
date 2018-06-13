var __extends = (this && this.__extends) || (function () {
    var extendStatics = Object.setPrototypeOf ||
        ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
        function (d, b) { for (var p in b) if (b.hasOwnProperty(p)) d[p] = b[p]; };
    return function (d, b) {
        extendStatics(d, b);
        function __() { this.constructor = d; }
        d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
    };
})();
var AUTO = (function () {
    function AUTO() {
        this.static = new Static();
        Data.Item.setStatic(this.static);
        var loading = document.createElement('dialog');
        loading.id = "loading-screen";
        loading.appendChild(this.static.loading);
        var body = document.getElementsByTagName('body')[0];
        body.appendChild(loading);
        loading.showModal();
        this._cars = [];
        this._bases = [];
        this._port = null;
        this.panel = document.getElementById('panel');
        this.config();
        this.requestForm = new Module.RequestForm(this.panel, this);
        this._overview = new Module.Overview(this.panel);
        this._stats = new Module.Stats(this.panel);
        this.update();
        loading.close();
        body.removeChild(loading);
        loading.removeChild(this.static.loading);
        window.onresize = function () {
            if (window.innerWidth < 766) {
                var map = document.getElementById('map');
                body.removeChild(map);
                body.appendChild(map);
            }
        };
    }
    Object.defineProperty(AUTO.prototype, "cars", {
        get: function () { return this._cars; },
        enumerable: true,
        configurable: true
    });
    Object.defineProperty(AUTO.prototype, "bases", {
        get: function () { return this._bases; },
        enumerable: true,
        configurable: true
    });
    Object.defineProperty(AUTO.prototype, "port", {
        get: function () { return this._port; },
        set: function (port) { this._port = port; },
        enumerable: true,
        configurable: true
    });
    Object.defineProperty(AUTO.prototype, "overview", {
        get: function () { return this._overview; },
        enumerable: true,
        configurable: true
    });
    Object.defineProperty(AUTO.prototype, "stats", {
        get: function () { return this._stats; },
        enumerable: true,
        configurable: true
    });
    AUTO.prototype.config = function () {
        var app = this;
        var request = new XMLHttpRequest();
        request.onreadystatechange = function () {
            if (this.readyState == 4 && this.status == 200) {
                var data = JSON.parse(request.responseText);
                app.area = data;
                app.initMap();
            }
        };
        request.open("GET", "/auto/config", true);
        request.send(null);
    };
    AUTO.prototype.initMap = function () {
        var poz = { lat: this.static.poznan.lat.valueOf(), lng: this.static.poznan.lng.valueOf() };
        this.map = new google.maps.Map(document.getElementById('map'), {
            zoom: 12,
            center: poz
        });
        this.areaPoly = new google.maps.Polygon({
            paths: this.area,
            strokeColor: '#000000',
            strokeOpacity: 0.9,
            strokeWeight: 1,
            fillColor: '#757575',
            fillOpacity: 0.1,
            clickable: false
        });
        this.areaPoly.setMap(this.map);
        this.markers = [];
    };
    AUTO.prototype.update = function (app) {
        if (app === void 0) { app = this; }
        var request = new XMLHttpRequest();
        request.onreadystatechange = function () {
            if (this.readyState == 4 && this.status == 200) {
                var data = JSON.parse(request.responseText);
                var _loop_1 = function (d) {
                    (function () {
                        var dest;
                        if (d.job.duringJob)
                            dest = d.job.end;
                        else
                            dest = "-";
                        for (var _i = 0, _a = app.cars; _i < _a.length; _i++) {
                            var car = _a[_i];
                            if (d.id == car.getId()) {
                                car.update(new Data.Position(d.pos.lat, d.pos.lng), d.status, d.battery, d.mileage, dest, d.warnings);
                                return;
                            }
                        }
                        app.cars.push(new Data.Car(d.id, d.status, new Data.Position(d.pos.lat, d.pos.lng), d.battery, d.mileage, dest, d.warnings, app.map));
                    })();
                };
                for (var _i = 0, _a = data.cars; _i < _a.length; _i++) {
                    var d = _a[_i];
                    _loop_1(d);
                }
                var _loop_2 = function (d) {
                    (function () {
                        for (var _i = 0, _a = app.bases; _i < _a.length; _i++) {
                            var base = _a[_i];
                            if (d.id == base.getId()) {
                                base.update(d.free, d.cars.docked, d.cars.reserved);
                                return;
                            }
                        }
                        app.bases.push(new Data.Base(d.id, new Data.Position(d.pos.lat, d.pos.lng), app.map, d.name, d.slots, d.free, d.cars.docked, d.cars.reserved));
                    })();
                };
                for (var _b = 0, _c = data.bases; _b < _c.length; _b++) {
                    var d = _c[_b];
                    _loop_2(d);
                }
                var port = data.port;
                if (app.port == null)
                    app.port = new Data.Port(port.id, new Data.Position(port.pos.lat, port.pos.lng), app.map, port.name, port.slots, port.free, port.cars.docked, port.cars.reserved);
                else
                    app.port.update(port.free, port.cars.docked, port.cars.reserved);
                app.overview.update(data.stats.active, data.stats.free, data.stats.busy, data.stats.preTime);
                app.stats.update(app.cars);
                setTimeout(function (_app) {
                    if (_app === void 0) { _app = app; }
                    _app.update(_app);
                }, 15000);
            }
        };
        request.open("GET", "/auto/update", true);
        request.send(null);
    };
    return AUTO;
}());
var Static = (function () {
    function Static() {
        this.poznan = new Data.Position(52.403113, 16.925905);
        this.autoFree = { url: 'auto/static/auto_free.png', labelOrigin: new google.maps.Point(0, -15) };
        this.autoTaken = { url: 'auto/static/auto_taken.png', labelOrigin: new google.maps.Point(0, -15) };
        this.autoBusy = { url: 'auto/static/auto_busy.png', labelOrigin: new google.maps.Point(0, -15) };
        this.base = { url: 'auto/static/base.png', labelOrigin: new google.maps.Point(0, -15) };
        this.port = { url: 'auto/static/port.png', labelOrigin: new google.maps.Point(0, -15) };
        this.loading = document.createElement('img');
        this.loading.src = "/auto/static/loading.gif";
        this.loading.id = 'loading-bar';
    }
    return Static;
}());
window.onload = function () { return new AUTO(); };
var Data;
(function (Data) {
    var Position = (function () {
        function Position(lat, lng) {
            this.lat = lat;
            this.lng = lng;
        }
        ;
        return Position;
    }());
    Data.Position = Position;
    var PositionsAB = (function () {
        function PositionsAB() {
        }
        return PositionsAB;
    }());
    Data.PositionsAB = PositionsAB;
    var Item = (function () {
        function Item(id, position, map, _icon) {
            this.id = id;
            this.position = position;
            this.map = map;
            this.marker = new google.maps.Marker({
                position: { lat: this.position.lat, lng: this.position.lng },
                map: map,
                animation: google.maps.Animation.DROP,
                icon: _icon
            });
        }
        Item.setStatic = function (data) { Item.staticData = data; };
        Item.prototype.updateMarker = function (infoContent) {
            google.maps.event.clearListeners(this.marker, 'click');
            this.marker.addListener('click', function () {
                (new google.maps.InfoWindow({ content: infoContent })).open(this.map, this);
            });
        };
        Item.prototype.getId = function () { return this.id; };
        ;
        Item.prototype.getPosition = function () { return this.position; };
        ;
        return Item;
    }());
    Data.Item = Item;
    var Car = (function (_super) {
        __extends(Car, _super);
        function Car(_id, status, _position, battery, mileage, destination, warnings, _map) {
            var _this = _super.call(this, _id, _position, _map, Car.staticData.autoFree) || this;
            _this.status = status;
            _this.battery = battery;
            _this.mileage = mileage;
            _this.destination = destination;
            _this.warnings = warnings;
            _this.updateMarker();
            return _this;
        }
        ;
        Car.prototype.updateMarker = function () {
            _super.prototype.updateMarker.call(this, "car: " + this.id +
                "<br>status: " + this.status +
                "<br>battery: " + Math.round(this.battery / 10000) + "%" +
                "<br>mileage: " + (this.mileage / 1000).toFixed(2) + "km" +
                "<br>destination: " + this.destination +
                "<br>warnings: " + this.getWarnigsStr());
            switch (this.status) {
                case 101:
                case 104:
                    this.marker.setIcon(Car.staticData.autoFree);
                    break;
                case 102:
                    this.marker.setIcon(Car.staticData.autoTaken);
                    break;
                case 103:
                case 105:
                    this.marker.setIcon(Car.staticData.autoBusy);
                    break;
            }
            if (this.status == 104 || this.status == 106 || this.status == 107)
                this.marker.setVisible(false);
            else
                this.marker.setVisible(true);
        };
        Car.prototype.update = function (newPosition, newStatus, newBattery, newMileage, newDest, newWarns) {
            if (this.position != newPosition) {
                this.position = newPosition;
                this.marker.setPosition({ lat: this.position.lat, lng: this.position.lng });
            }
            this.status = newStatus;
            this.battery = newBattery;
            this.mileage = newMileage;
            this.destination = newDest;
            this.warnings = newWarns;
            this.updateMarker();
        };
        Car.prototype.getStatus = function () { return this.status; };
        ;
        Car.prototype.getBattery = function () { return this.battery; };
        Car.prototype.getMileage = function () { return this.mileage; };
        Car.prototype.getDestination = function () { return this.destination; };
        Car.prototype.getWarnigsStr = function () {
            var w = '';
            for (var _i = 0, _a = this.warnings; _i < _a.length; _i++) {
                var i = _a[_i];
                w += i + ' ';
            }
            return w;
        };
        return Car;
    }(Item));
    Data.Car = Car;
    var Base = (function (_super) {
        __extends(Base, _super);
        function Base(_id, _position, _map, name, slots, free, docked, reserved, icon) {
            if (icon === void 0) { icon = Car.staticData.base; }
            var _this = _super.call(this, _id, _position, _map, icon) || this;
            _this.name = name;
            _this.slots = slots;
            _this.free = free;
            _this.docked = docked;
            _this.reserved = reserved;
            _this.updateMarker();
            return _this;
        }
        ;
        Base.prototype.updateMarker = function () {
            _super.prototype.updateMarker.call(this, 'base: ' + this.name +
                '<br>id: ' + this.id +
                '<br>free slots: ' + this.free + '/' + this.slots +
                '<br>docked cars: ' + this.docked.toString() +
                '<br>reserved cars: ' + this.reserved.toString());
        };
        Base.prototype.update = function (newFree, newDocked, newReserved) {
            this.free = newFree;
            this.docked = newDocked;
            this.reserved = newReserved;
            this.updateMarker();
        };
        return Base;
    }(Item));
    Data.Base = Base;
    var Port = (function (_super) {
        __extends(Port, _super);
        function Port(_id, _position, _map, name, slots, free, docked, reserved) {
            return _super.call(this, _id, _position, _map, name, slots, free, docked, reserved, Car.staticData.port) || this;
        }
        ;
        return Port;
    }(Base));
    Data.Port = Port;
})(Data || (Data = {}));
var Module;
(function (Module_1) {
    var Module = (function () {
        function Module(panel, name, iconUrl, id) {
            this.container = document.createElement('div');
            this.container.className = 'module';
            this.container.id = id;
            panel.appendChild(this.container);
            this.header = document.createElement('div');
            this.header.className = 'header';
            var icon = document.createElement('div');
            icon.className = 'icon';
            icon.style.backgroundImage = 'url("auto/static/' + iconUrl + '")';
            this.header.appendChild(icon);
            var title = document.createElement('h3');
            title.textContent = name;
            this.header.appendChild(title);
            this.container.appendChild(this.header);
        }
        return Module;
    }());
    var RequestForm = (function (_super) {
        __extends(RequestForm, _super);
        function RequestForm(panel, app) {
            var _this = _super.call(this, panel, 'Order ride', 'order-icon.png', 'request-form') || this;
            _this.app = app;
            _this.form = document.createElement('form');
            _this.form.id = 'order-form';
            _this.validation = { a: false, b: false };
            _this.aInput = document.createElement('input');
            _this.aInput.type = 'text';
            _this.aInput.placeholder = 'Enter origin';
            _this.form.appendChild(_this.aInput);
            _this.aStatus = document.createElement('div');
            _this.aStatus.className = 'status-icon';
            _this.form.appendChild(_this.aStatus);
            _this.bInput = document.createElement('input');
            _this.bInput.type = 'text';
            _this.bInput.placeholder = 'Enter destination';
            _this.form.appendChild(_this.bInput);
            _this.bStatus = document.createElement('div');
            _this.bStatus.className = 'status-icon';
            _this.form.appendChild(_this.bStatus);
            _this.orderButton = document.createElement('input');
            _this.orderButton.type = 'button';
            _this.orderButton.value = 'Request';
            _this.orderButton.onclick = function () { _this.sendOrder(_this); };
            _this.form.appendChild(_this.orderButton);
            _this.info = document.createElement('dialog');
            _this.info.close();
            _this.infoContent = document.createElement('section');
            _this.info.appendChild(_this.infoContent);
            var button = document.createElement('button');
            button.onclick = function () { _this.info.close(); _this.infoContent.removeChild(_this.infoContent.firstChild); };
            button.textContent = 'close';
            _this.info.appendChild(button);
            _this.container.appendChild(_this.form);
            _this.container.appendChild(_this.info);
            _this.initAutocomplete();
            _this.aPosition = new Data.Position(0, 0);
            _this.bPosition = new Data.Position(0, 0);
            _this.checkValidity();
            return _this;
        }
        RequestForm.prototype.checkValidity = function (val) {
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
        };
        RequestForm.prototype.resetInputs = function () {
            this.aInput.value = "";
            this.aStatus.className = "status-icon";
            this.bInput.value = "";
            this.bStatus.className = "status-icon";
            this.checkValidity({ a: false, b: false, val: undefined });
        };
        RequestForm.prototype.initAutocomplete = function () {
            var _this = this;
            this.aSearchBox = new google.maps.places.SearchBox(this.aInput);
            var callback = function (form, searchBox, position, status, validation) {
                var places = searchBox.getPlaces();
                if (places.length == 0)
                    return;
                if (places.length == 1) {
                    position.lat = places[0].geometry.location.lat();
                    position.lng = places[0].geometry.location.lng();
                    var request_1 = new XMLHttpRequest();
                    request_1.onreadystatechange = function () {
                        if (this.readyState == 4 && this.status == 200) {
                            switch (JSON.parse(request_1.responseText).response) {
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
                    };
                    request_1.open("POST", "/auto/request/isAvailable", true);
                    request_1.send(JSON.stringify(position));
                    return;
                }
                window.alert("Input correct place");
            };
            this.aSearchBox.addListener('places_changed', function (form) {
                if (form === void 0) { form = _this; }
                callback(form, form.aSearchBox, form.aPosition, form.aStatus, { a: true, b: false });
            });
            this.bSearchBox = new google.maps.places.SearchBox(this.bInput);
            this.bSearchBox.addListener('places_changed', function (form) {
                if (form === void 0) { form = _this; }
                callback(form, form.bSearchBox, form.bPosition, form.bStatus, { a: false, b: true });
            });
        };
        RequestForm.prototype.sendOrder = function (form) {
            if (form === void 0) { form = this; }
            var placeholder = document.createElement('img');
            form.infoContent.appendChild(form.app.static.loading);
            form.info.showModal();
            var order = new Data.PositionsAB();
            order.A = form.aPosition;
            order.B = form.bPosition;
            var orderJSON = JSON.stringify(order);
            var request = new XMLHttpRequest();
            request.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    var data = JSON.parse(request.responseText);
                    var content = document.createElement('div');
                    content.classList.add('order-info');
                    if (data.status == 1001) {
                        var img = document.createElement('img');
                        img.src = "/auto/static/car.png";
                        content.appendChild(img);
                        var heading = document.createElement('h3');
                        heading.textContent = "Ride details: ";
                        content.appendChild(heading);
                        var p = document.createElement('p');
                        p.classList.add('car');
                        p.textContent = "assinged car: " + data.job.id;
                        content.appendChild(p);
                        p = document.createElement('p');
                        p.classList.add('pre');
                        p.textContent = "arrive in: " + (data.job.preDuration / 60).toFixed(0) + 'min';
                        content.appendChild(p);
                        p = document.createElement('p');
                        p.classList.add('place');
                        p.textContent = "from: " + data.job.origin;
                        content.appendChild(p);
                        p = document.createElement('p');
                        p.classList.add('place');
                        p.textContent = "to: " + data.job.destination;
                        content.appendChild(p);
                        p = document.createElement('p');
                        p.classList.add('time');
                        p.textContent = "travel time: around " + (data.job.duration / 60).toFixed(0) + 'min';
                        content.appendChild(p);
                    }
                    else {
                        content.textContent = 'Your order was cancelled. Try again';
                    }
                    form.infoContent.removeChild(form.infoContent.firstChild);
                    form.infoContent.appendChild(content);
                }
            };
            request.open("POST", "/auto/request/route", true);
            request.send(orderJSON);
            this.resetInputs();
        };
        return RequestForm;
    }(Module));
    Module_1.RequestForm = RequestForm;
    var Overview = (function (_super) {
        __extends(Overview, _super);
        function Overview(panel) {
            var _this = _super.call(this, panel, 'Overview', 'overview-icon.png', 'overview') || this;
            _this.activeCars = document.createElement('div');
            _this.activeCars.classList.add('number');
            _this.container.appendChild(_this.activeCars);
            var text = document.createElement('p');
            text.textContent = 'active cars';
            _this.container.appendChild(text);
            _this.freeCars = document.createElement('div');
            _this.freeCars.classList.add('number');
            _this.container.appendChild(_this.freeCars);
            text = document.createElement('p');
            text.textContent = 'free cars';
            _this.container.appendChild(text);
            _this.busyCars = document.createElement('div');
            _this.busyCars.classList.add('number');
            _this.container.appendChild(_this.busyCars);
            text = document.createElement('p');
            text.textContent = 'busy cars';
            _this.container.appendChild(text);
            _this.averagePre = document.createElement('div');
            _this.averagePre.classList.add('number');
            _this.container.appendChild(_this.averagePre);
            text = document.createElement('p');
            text.textContent = 'avg. waiting time';
            _this.container.appendChild(text);
            return _this;
        }
        Overview.prototype.update = function (active, free, busy, averagePre) {
            this.activeCars.textContent = active.toString();
            this.freeCars.textContent = free.toString();
            this.busyCars.textContent = busy.toString();
            this.averagePre.textContent = (averagePre / 60).toFixed(0).toString() + 'min';
        };
        return Overview;
    }(Module));
    Module_1.Overview = Overview;
    var Element = (function () {
        function Element(car) {
            this.car = car;
            this._li = document.createElement('li');
            this.id = document.createElement('div');
            this.id.classList.add('id');
            this.id.classList.add('element');
            this._li.appendChild(this.id);
            this.status = document.createElement('div');
            this.status.classList.add('status');
            this.status.classList.add('element');
            this._li.appendChild(this.status);
            this.battery = document.createElement('div');
            this.battery.classList.add('battery');
            this.battery.classList.add('element');
            this._li.appendChild(this.battery);
            this.destination = document.createElement('div');
            this.destination.classList.add('destination');
            this.destination.classList.add('element');
            this._li.appendChild(this.destination);
            this.mileage = document.createElement('div');
            this.mileage.classList.add('mileage');
            this.mileage.classList.add('element');
            this._li.appendChild(this.mileage);
            this.warnings = document.createElement('div');
            this.warnings.classList.add('warnings');
            this.warnings.classList.add('element');
            this._li.appendChild(this.warnings);
            this.update();
        }
        Object.defineProperty(Element.prototype, "li", {
            get: function () { return this._li; },
            enumerable: true,
            configurable: true
        });
        Element.prototype.getId = function () { return this.car.getId(); };
        Element.prototype.update = function () {
            this.id.textContent = this.car.getId().toString();
            this.status.textContent = this.car.getStatus().toString();
            this.battery.textContent = (this.car.getBattery() / 10000).toFixed(0).toString() + '%';
            this.destination.textContent = this.car.getDestination();
            this.mileage.textContent = (this.car.getMileage() / 1000).toFixed(0).toString() + 'km';
            var w = this.car.getWarnigsStr();
            this.warnings.textContent = w == '' ? '-' : w;
        };
        return Element;
    }());
    var Stats = (function (_super) {
        __extends(Stats, _super);
        function Stats(panel) {
            var _this = _super.call(this, panel, 'Stats', 'stats-icon.png', 'stats') || this;
            var header = document.createElement('h3');
            var temp = document.createElement('div');
            temp.classList.add('id');
            temp.classList.add('element');
            temp.classList.add('list-header');
            temp.textContent = 'id';
            _this.container.appendChild(temp);
            temp = document.createElement('div');
            temp.classList.add('status');
            temp.classList.add('element');
            temp.classList.add('list-header');
            temp.textContent = 'status';
            _this.container.appendChild(temp);
            temp = document.createElement('div');
            temp.classList.add('battery');
            temp.classList.add('element');
            temp.classList.add('list-header');
            temp.textContent = 'batt.';
            _this.container.appendChild(temp);
            temp = document.createElement('div');
            temp.classList.add('destination');
            temp.classList.add('element');
            temp.classList.add('list-header');
            temp.textContent = 'destination';
            _this.container.appendChild(temp);
            temp = document.createElement('div');
            temp.classList.add('mileage');
            temp.classList.add('element');
            temp.classList.add('list-header');
            temp.textContent = 'mil.';
            _this.container.appendChild(temp);
            temp = document.createElement('div');
            temp.classList.add('warnings');
            temp.classList.add('element');
            temp.classList.add('list-header');
            temp.textContent = 'warnings';
            _this.container.appendChild(temp);
            _this.list = document.createElement('ul');
            _this.container.appendChild(_this.list);
            _this.elements = [];
            return _this;
        }
        Stats.prototype.update = function (cars) {
            var _this = this;
            var _loop_3 = function (car) {
                (function () {
                    for (var _i = 0, _a = _this.elements; _i < _a.length; _i++) {
                        var element = _a[_i];
                        if (element.getId() == car.getId()) {
                            element.update();
                            return;
                        }
                    }
                    var newElement = new Element(car);
                    _this.list.appendChild(newElement.li);
                    newElement.update();
                    _this.elements.push(newElement);
                })();
            };
            for (var _i = 0, cars_1 = cars; _i < cars_1.length; _i++) {
                var car = cars_1[_i];
                _loop_3(car);
            }
        };
        return Stats;
    }(Module));
    Module_1.Stats = Stats;
})(Module || (Module = {}));
