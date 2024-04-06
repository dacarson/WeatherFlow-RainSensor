  document.addEventListener('DOMContentLoaded', function() {
    const swapLink = document.getElementById('swapLink');
    const configuration = document.getElementById('configuration');
    const status = document.getElementById('status');

    swapLink.addEventListener('click', function(e) {
      e.preventDefault(); // Prevents the link from following the URL

      if (configuration.style.display === 'none') {
        configuration.style.display = 'block';
        status.style.display = 'none';
	swapLink.innerHTML="Status";
      } else {
        configuration.style.display = 'none';
        status.style.display = 'block';
	swapLink.innerHTML="Configuration";
      }
    });
  });

