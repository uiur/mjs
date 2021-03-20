Array.prototype.forEach = function (fn) {
  for (var i = 0; i < this.length; i = i + 1) {
    fn(this[i]);
  }
};

var a = [1, 2, 3];
a.forEach(function (n) {
  console.log(n);
});
