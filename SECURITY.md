# Security Policy

## Supported Versions

As an open source product, we will only provide security patches for the latest major version. Older versions will not receive retroactive security patches.

## Bug Bounty Program

CMRX does not currently operate a bug bounty program. We do not offer monetary rewards for vulnerability reports.

We appreciate responsible disclosure and will acknowledge contributors in our release notes (with permission), but submitting a report does not guarantee or imply any monetary compensation.

## Reporting Security Issues

### case1: Report via Email

If you discover a security vulnerability, please report it to us in the following manner:

1. **Email us** at security [near] cmrxrtos.org. **Please do not create a public GitHub issue.**
2. Include as much detail as possible, including steps to reproduce the vulnerability, potential impact, and any other relevant information.
3. We will acknowledge your email within 3 business days and work with you to understand the issue and address it promptly.

### case2: Report via GitHub Private vulnerability reporting

Out team and community take security bugs in seriously. We appreciate your efforts to responsibly disclose your findings, and will make every effort to acknowledge your contributions.

To report a security issue, please use the GitHub Security Advisory ["Report a Vulnerability"](https://github.com/ventZl/cmrx/security/advisories/new) tab. **Do not open up a GitHub issue.**

Our team will send a response indicating the next steps in handling your report. After the initial reply to your report, the team will keep you informed of the progress towards a fix and full announcement, and may ask for additional information or guidance.

## Handling Security Issues

We follow a responsible disclosure process:

1. We will investigate the reported vulnerability and work on a fix.
2. A fix will be developed, tested, and incorporated into the project.
3. Once the fix is ready, we will release a new version of the project with a detailed release note.
4. We will notify the reporter about the fix and acknowledge their contribution in the release notes, if they wish to be credited.

## Security Best Practices

To ensure the security of our project, we are committed the following best practices:

1. **Static code analysis**: Each code change is a subject of automated static code analysis
2. **Use secure coding practices**: Follow best practices for secure coding to minimize vulnerabilities.
3. **Stay informed**: Keep up to date with the latest security news and advisories related to the technologies used in this project.
4. **No BLOBs in repository**: Don't include opaque BLOBs in repository whose content cannot be audited

## Out of Scope

The following items are generally considered out of scope:

- Denial of service attacks
- Reports from automated tools without demonstrated impact

## Responsible Disclosure Guidelines

We kindly ask reporters to:

- Allow reasonable time for us to investigate and address the issue before public disclosure
- Avoid ultimatum-style deadlines or threats of public disclosure as a negotiation tactic
- Provide sufficient technical details (reproduction steps, PoC, headers, etc.) to validate the issue

## Contact

For any other security-related inquiries, please contact us at security [near] cmrxrtos.org.

Thank you for helping us keep our project secure!
