export default function delay(milliseconds = 1500) {
  return new Promise(function(resolve) {
    setTimeout(resolve, milliseconds);
  })
}
