class EventEmitter {
  constructor() {
    this._listeners = {};
  }

  addListener(event, listener) {
    if (!this._listeners[event]) {
      this._listeners[event] = [];
    }
    this._listeners[event].push(listener);
  }

  removeListener(event, listener) {
    if (!this._listeners[event]) return;
    this._listeners[event] = this._listeners[event].filter(l => l !== listener);
  }

  emit(event, ...args) {
    if (!this._listeners[event]) return;
    this._listeners[event].forEach(l => l(...args));
  }
}

var eventDispatcher = new EventEmitter();

export function sharedEventDispatcher() {
    return eventDispatcher;
}
