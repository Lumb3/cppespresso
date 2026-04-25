
async function fetchAPI() {
    try {
        const response = await fetch(`http://localhost:8080/data`);
        if (!response.ok) {
            console.error("Failed to fetch the data");
        }
        console.log(response); // full response
        const data = await response.json(); // parsed body data
        console.log(data);
    } catch(e) {
        console.error("Error occurred: ", e);
    }
}
function Main() {
    fetchAPI().then(() => {
        console.log("Finished");
    });
}

Main();