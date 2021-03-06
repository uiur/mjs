function swap(items, firstIndex, secondIndex){
  var temp = items[firstIndex];
  items[firstIndex] = items[secondIndex];
  items[secondIndex] = temp;
}

var a = [3, 2, 1];
swap(a, 0, 1);
console.log(a);
