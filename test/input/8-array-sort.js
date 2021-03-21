function sort(items) {
  for (var i = 0; i < items.length; i = i + 1) {
    var j = 0;
    var stop = items.length - 1;
    for (; j < stop; j = j + 1){
      if (items[j] > items[j + 1]){
        var temp = items[j];
        items[j] = items[j + 1];
        items[j + 1] = temp;
      }
    }
  }

  return items;
}

Array.prototype.sort = function () {
  return sort(this);
};

console.log([3, 2, 1].sort());
