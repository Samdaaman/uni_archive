async function sleep(milliseconds) {
    const date = Date.now();
    let currentDate = null;
    do {
      currentDate = Date.now();
    } while (currentDate - date < milliseconds);
    return;
}

function wait(ms, data) {
    return new Promise( resolve => setTimeout(resolve.bind(this, data), ms), reject => console.log("Rejected") );
}
  
  /** 
   * This will run in series, because 
   * we call a function and immediately wait for it's result, 
   * so this will finish in 1s.
   */
  async function series() {
    return {
      result1: await wait(500, 'seriesTask1'),
      result2: await wait(500, 'seriesTask2'),
    }
  }
  
  /** 
   * While here we call the functions first,
   * then wait for the result later, so 
   * this will finish in 500ms.
   */
  async function parallel() {
    const task1 = wait(500, 'parallelTask1');
    const task2 = wait(500, 'parallelTask2');
  
    return {
      result1: await task1,
      result2: await task2,
    }
  }
  
  async function taskRunner(fn, label) {
    console.log(`Task ${label} starting...`);
    let result = await fn();
    console.log(`Task ${label} finished in  miliseconds with,`, result);
  }
  
  void taskRunner(series, 'series');
  void taskRunner(parallel, 'parallel');

  console.log("Waiting for a few secs");
  sleep(3000);
  console.log("End of script");