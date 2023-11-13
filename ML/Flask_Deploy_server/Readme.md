Flask and gunicorn tutorial
===========================

The default development server for flask is not suited for production environments. Gunicorn is a simple WSGI client written in pure python. This repository serves as a simple example of connecting the two.

[This answer](https://serverfault.com/a/331263) on StackOverflow explains why something like Gunicorn is needed.

## Simpler answer on why Gunicorn is needed
### Analogy: Hosting a Party
Imagine you are hosting a party (your Flask application) at your house (the server). When you invite a few friends over (a few users accessing your app), it's easy to manage everything by yourself (using the built-in Flask server). This is like running flask run in a development environment: it's great for a small number of guests, but not for a large crowd.

Now, imagine your party becomes popular, and suddenly you have a lot more guests (users). Managing everyone by yourself becomes impractical. This is where Gunicorn comes in. It's like hiring a team of professional party planners and hosts (Gunicorn's worker processes) to help you manage the guests. They can efficiently handle multiple tasks simultaneously, like taking coats, serving drinks, and managing music, all at the same time.

### Simple Terms Explanation
- **Handling Multiple Requests:** The Flask development server is designed for testing and development. It can generally only handle one request at a time. In production, you might have many users making requests simultaneously. Gunicorn, on the other hand, can handle multiple requests at the same time efficiently.
- **Stability and Robustness:** Gunicorn is more stable and can manage resources better. It's designed to ensure that if one part of your application has a problem, it doesn't bring down the entire app. This is crucial for a production environment where uptime and reliability are important.
- **Performance:** Gunicorn can significantly improve the performance of your Flask application in a production environment. It does this by managing worker processes, which allows your application to handle more traffic and process requests faster.
- **Security:** Running a Flask app in production using the development server can also pose security risks. Gunicorn provides a more secure environment for hosting your Flask application.

More on Gunicorn can be found [here](http://docs.gunicorn.org/en/latest/run.html)

> Please run all below scripts in the root directory!

Running flask server
====================

`export FLASK_APP="app.main:create_app"`

`flask run`

Running gunicorn server
=======================

`gunicorn -w 4 --reload -b localhost:5000 "app.main:app"`

Running from wsgi
=======================

`gunicorn --bind 0.0.0.0:5000 wsgi:app`