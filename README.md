Disables TLS in Riot XMPP so you can proxy chat without needing valid cert.

# How to use
1. Open Riot Client with your xmpp Proxy
2. Either auto inject the DLL or do it with process hacker into RiotClientServices.exe
3. DLL must be injected before it sends player config request, if you inject to late, sign out and sign back into your Riot account for changes to take effect.
