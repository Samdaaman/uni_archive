# SENG365 2020 Assignment 1

## Overview of the continuous deployment process

1. You hackity hack...
2. You commit your changes to your git repo on `eng-git.canterbury.ac.nz`.
3. That triggers the GitLab CI runner, which starts a GitLab docker executor on the build VM to build your application within a docker container.
4. The executor runs the build defined in your `.gitlab-ci.yml` file. Docker compose build follows the `build` steps from `docker-compose.yml` to build the application.
5. If the `build` step succeeds, the CI runner on the VM then runs the scripts in the `deploy` section of `.gitlab-ci.yml` in the same way. This calls `docker compose down` to stop any previous version of your app, and then `docker compose up` to start your server, mapping your unique `SENG365_PORT` on the VM to port `4941` in the container.
6. Your application is now up-and-running at `csse-s365.canterbury.ac.nz:{SENG365_PORT}` (e.g. http://csse-s365.canterbury.ac.nz:4001).
7. The CI runner then runs the verify scripts in .gitlab-ci.yml, checking that the server has not immediately crashed, and retrieving the logs so far to provide debug information.

Logs for the entire CI process are available in the CI / CD → Pipelines section in your GitLab repo.

Note: The value of ```SENG365_PORT``` is defined for you when the skeleton project was added into your eng-git repo. You can find the value for your project by going to the eng-git project and then: Settings → CI / CD → Variables → Expand → Reveal values.

## Running locally

1. Use `npm install` to populate the `node_modules/` directory with up-to-date packages
2. Create a file called `.env`, following the instructions in the section below
3. Go to https://dbadmin.csse.canterbury.ac.nz and create a database with the name that you set in the `.env` file
2. Run `npm run start` or `npm run debug` to start the server
3. The server will be accessible on `localhost:4941`

### `.env` file
Create a `.env` file in the root directory of this project including the following information (note that you will need to create the database first in phpMyAdmin):

```
SENG365_MYSQL_HOST=db2.csse.canterbury.ac.nz
SENG365_MYSQL_USER={your usercode}
SENG365_MYSQL_PASSWORD={your password}
SENG365_MYSQL_DATABASE={a database starting with your usercode then an underscore}
```

For example:
```
SENG365_MYSQL_HOST=db2.csse.canterbury.ac.nz
SENG365_MYSQL_USER=abc123
SENG365_MYSQL_PASSWORD=password
SENG365_MYSQL_DATABASE=abc123_main
```

## Automated testing

There is a webpage set up for you to run some tests on your application. These are a subset of the full set of tests that will be used to mark your assignment. (Note that a different set of data will be used for marking.)

### On your deployed application

1. Go to http://csse-s365.canterbury.ac.nz
2. Enter the url `csse-s365.canterbury.ac.nz` and your unique `SENG365_PORT` in order to run the API tests on the latest build of your server
3. The test runner will generate a report of the results for you 

### On your local application

This is only possible when using a lab machine (or by SSHing into your uni account, see pages 6 and 7 of [this CompSoc handout](https://drive.google.com/file/d/0B8dalXEwJSiZYnN5OV8xMFVaSjg/view) for help doing that).

Follow the same steps as for your deployed application, but with a different URL: instead of `csse-s365.canterbury.ac.nz`, run `curl ifconfig.me; echo` in a terminal and use the IP address it outputs and port `4941`.

## Manual testing, with Postman

There is a Postman collection, in `app/resources/postman`, with request(s) for each endpoint. You can use this to query your application.

To import the collection:
1. Click Import (top left)
2. Click Choose Files
3. Select `Petitions site.postman_collection.json`

To import the environments:
4. Click the gear icon (⚙️)
5. Click Import
6. Click Choose Files
6. Select the three files that end in `.postman_environment.json`
7. Click Petitions: deployed application
8. Change the current value so that it uses your `SENG365_PORT` instead of `4001`
9. Click Update

To choose which application you send your requests to, select the corresponding environment from the dropdown in the top right (by default, it will be "No Environment"). This will set the `BASE_URL` variable.

* "Petitions: deployed application" will use the version of your application that is deployed to the docker container.
* "Petitions: localhost" will use the locally running application (on port 4941).
* "Petitions: reference server" will use the reference server.

To test the PUT photo endpoints, you will need to copy the images in `app/resources/postman/files` into your working directory. By default, this will be `~/Postman/files`, but you can check by clicking the spanner icon in the top right, clicking Settings, the scrolling down to Working Directory.

### How it works: your user ID, token, etc.

Some of the Postman requests, such as POST /users/login, have scripts included, in the Tests tab. These set global variables, such as your user ID and the auth token, which are then used in other requests. For example, `auth_token` is used in the `X-Authorization` header of PATCH /users/:id. The POST /users/logout request then has a script to delete the user ID and token.

## Storing photos

You should set up your application to store files in the `storage/photos` directory; this is where the photos used for the sample data are copied into when you run a `/resample` or `/reload` request. There will initially be a file called `.gitkeep` in there; this is just so that the directory gets included in the git repository (see https://stackoverflow.com/q/7229885/8355496 for more information).

## Debugging your deployed application

To see the logs (e.g. from `console.log` calls) from the application that is running on csse-s365.canterbury.ac.nz, go to your project on eng-git, then:
1. Open CI / CD → Pipelines
2. In the most recent (top) pipeline, click the third of the three green ticks ✔✔✔ (if the pipeline passed)
3. Click `get-logs`
4. Click the Retry button in the top-right

## Setting up a database connection in WebStorm

To enable autocomplete for database tables, columns, etc., you can connect WebStorm to the `db2` database by following the instructions at https://www.jetbrains.com/help/idea/connecting-to-a-database.html#connect-to-mysql-database. The details to enter are:

* Host: `db2.csse.canterbury.ac.nz`
* Port: `3306`
* User: (your usercode)
* Password: (your password)
* Database: (your database, as in `.env`)
