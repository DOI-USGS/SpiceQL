# Build SpiceQL
To get started, check out the [steps](https://github.com/DOI-USGS/SpiceQL/blob/main/README.md#building-the-library) to create a local build of SpiceQL.

# Pick an Issue
Next, you are ready to pick an issue to start working on. Head over to the [issues page](https://github.com/DOI-USGS/SpiceQL/issues), where you will find bugs, questions, and feature requests. This will show you all of the open issues that will be a good place to start developing. Pick an issue that you may want to work on, click on the issue title to go to its page,  and assign yourself to that issue by clicking on “assign yourself” under the “Assignees” section to the right of the issue description. Now, you are ready to start developing.

# Document Your Changes
Once you have completed the ticket, i.e., by fixing an issue or adding a feature, make sure you have updated any documentation and the CHANGELOG.md when applicable.

# Test Your Changes
You will also need to test the code, update any tests that are now failing because of the changes, or add new tests that cover your changes. Check out the [Building The Library](https://github.com/DOI-USGS/SpiceQL/blob/main/README.md#building-the-library) part of the README to see how to run SpiceQL tests.

# Create a Pull Request
Next, commit your changes to your local branch with the command 
`git commit -m “<message>”` where <message> is your commit message. Then, push up your changes to your fork 
`git push origin <branch_name>` where `<branch_name>` is the name of your local branch you made your changes on. Finally, head over to your fork, click on the “Branch” dropdown, select the branch `<branch_name>` from the dropdown, and click “New Pull Request”. Give your PR a descriptive but brief title and fill out the description skeleton. After submitting your PR, wait for someone to review it, and make any necessary changes.
