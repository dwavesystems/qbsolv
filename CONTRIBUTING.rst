.. _contributing:

We, the qbsolv developers, expect a variety of contributions, from reporting of bugs or
suggestions for examples or tutorials to development of improvements or alternatives to
qbsolv's core algorithms.  Here we describe how we plan to interact with contributors
making such contributions, though that plan will of course change as those contributions
occur.

Issues
======

Bugs or anomalies in the behavior of the tool, issues with its installation, requests
for changes in documentation, design and development issues, and feature requests will
be tracked via GitHub's Issues mechanism.

When reporting a bug, please provide the following info if appropriate:

    * What are the steps to reproduce the bug?  If possible, the simplest such set of steps
      is best.
    * Does the bug still happen using the latest version?
    * What qbsolv version and OS are you using?

Contributions
=============

We believe that qbsolv will make the fastest progress to being a robust metaheuristic
solver capable of quantum acceleration by being widely used both by end-users solving
problems and by metaheuristic algorithm developers exploring new metaheuristic algorithms.

In general, you may want to think about starting your contributions gradually, using
qbsolv and reporting any strengths and weaknesses (e.g., bugs, documentation improvements)
you encounter.  This will help you build relationships with the qbsolv developers.  And
don't forget that contributions can be other than just code or documentation;  creating
an example or a tutorial helps new users come up to speed and is often high value.

We expect to incorporate promising and proven algorithmic changes into the master code base.
This wide use by algorithm developers requires a balance between accepting changes and
maintaining a stable tool for end-users. Over time we expect to have processes for both
building and regression testing (both correctness and performance) that we will expect new
changes to pass before being considered for inclusion.

Submitting A Contribution
=========================

For now, we accept a contribution as a Pull Request (PR) on GitHub, though this may change.
Please follow these steps:

    * If your change is substantial, first create a feature request to start a discussion
      with the developers to ensure your intent aligns with qbsolv plans.
    * A PR should have a clear purpose and do exactly one thing.  This enables the rest
      of the process to be crisp.
    * Each commit in PR should be an atomic change representing one step in development.
    * As appropriate, please explain anything that is not obvious from the code; this could be
      in comments, commit messages, or the PR description.
    * Sign your patch via the sign-off line, often created by git commit -s.  Your sign-off
      certifies that you wrote the patch or otherwise have the right to pass it along as
      an open-source patch; see the full text of the certificate just below. Your
      sign-off line might look like Signed-off-by: Abby Smith <abby.smith@mail.com>
      Please use your real name, not a pseudonym.  This project does not accept
      anonymous contributions.

License
=======

qbsolv is licensed under the terms in LICENSE.
By contributing to the project, you agree to the terms of the license and to release your
contribution under those terms.
