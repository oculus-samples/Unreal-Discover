Go to [Epic Developer Website](https://dev.epicgames.com) and click on Dev Portal.
Login or Create an Account.

<div style="text-align: center; padding: 10pt;"><img src="./Media/EOS_Config/twofactor.png" align="middle" width="450"></div>
<div style="text-align: center; padding: 10pt;"><img src="./Media/EOS_Config/acount2fa.png" align="middle" width="450"></div>

Set up a Two-Factor Authentication by clicking account mangement if you haven't already. 
You can use a phone number or an email. Once finished click *I've Enabled 2FA*

<div style="text-align: center; padding: 10pt;"><img src="./Media/EOS_Config/createorg.png" align="middle" width="450"></div>

Enter an orginzation name and select your country, then press continue.

<div style="text-align: center; padding: 10pt;"><img src="./Media/EOS_Config/createproduct.png" align="middle" width="450"></div>
<div style="text-align: center; padding: 10pt;"><img src="./Media/EOS_Config/createproduct2.png" align="middle" width="450"></div>

Click create product. Enter a product name, click continue and upload a cover image if you want to.

<div style="text-align: center; padding: 10pt;"><img src="./Media/EOS_Config/productsettings.png" align="middle" width="450"></div>

Click the gear (settings) next to the product name.

<div style="text-align: center; padding: 10pt;"><img src="./Media/EOS_Config/productsettings2.png" align="middle" width="450"></div>
<div style="text-align: center; padding: 10pt;"><img src="./Media/EOS_Config/agreements.png" align="middle" width="250"></div>

Click clients, then review & accept.

<div style="text-align: center; padding: 10pt;"><img src="./Media/EOS_Config/agreements2.png" align="middle" width="450"></div>

Scroll all the way down and accept should be clickable.

<div style="text-align: center; padding: 10pt;"><img src="./Media/EOS_Config/agreements3.png" align="middle" width="450"></div>

Click clients again, go to the Anti-Cheat Servers and Epic Account Serves licenses and accept them.
When finished click on Dashboard, go back to your product settings and click clients again.

<div style="text-align: center; padding: 10pt;"><img src="./Media/EOS_Config/clients.png" align="middle" width="450"></div>

Click *Add new client*.

<div style="text-align: center; padding: 10pt;"><img src="./Media/EOS_Config/clients2.png" align="middle" width="350"></div>

Enter a client name and then click *Add new client policy*

<div style="text-align: center; padding: 10pt;"><img src="./Media/EOS_Config/clientpolicy.png" align="middle" width="350"></div>
<div style="text-align: center; padding: 10pt;"><img src="./Media/EOS_Config/clientpolicytype.png" align="middle" width="350"></div>

The client policy is what the client can do with the online service.
Enter a client policy name and select one of the client policy types or create a custom one.

<div style="text-align: center; padding: 10pt;"><img src="./Media/EOS_Config/custompolicy.png" align="middle" width="350"></div>


Below is what I have configured:

<div style="text-align: center; padding: 10pt;"><img src="./Media/EOS_Config/CP0.png" align="middle" width="350"></div>

<div style="text-align: center; padding: 10pt;"><img src="./Media/EOS_Config/CP1.png" align="middle" width="350"></div>

<div style="text-align: center; padding: 10pt;"><img src="./Media/EOS_Config/CP2.png" align="middle" width="350"></div>

<div style="text-align: center; padding: 10pt;"><img src="./Media/EOS_Config/CP3.png" align="middle" width="350"></div>

<div style="text-align: center; padding: 10pt;"><img src="./Media/EOS_Config/CP4.png" align="middle" width="350"></div>


If you selected *Create a custom policy*, 
then click on user required to require a logged in user to execute actions with that policy.
To get sharedspaces to work properly
Lobbies needs to be selected and all of the actions nees to be checked.
When finished click *Add new client policy*. Then click *Add new client*.

<div style="text-align: center; padding: 10pt;"><img src="./Media/EOS_Config/productsettingsupdate.png" align="middle" width="450"></div>

When you go back to Product settings you should now see a client ID and a client secret.

<div style="text-align: center; padding: 10pt;"><img src="./Media/EOS_Config/EAS.png" align="middle" width="450"></div>
<div style="text-align: center; padding: 10pt;"><img src="./Media/EOS_Config/linkedclient.png" align="middle" width="450"></div>

Go to Epic Account Services and click Linked Clients and select your created client and Save changes.
Go to Permissions and Save changes.

<div style="text-align: center; padding: 10pt;"><img src="./Media/EOS_Config/productsettingsupdate2.png" align="middle" width="450"></div>

When you go back to Product settings, you should now have an Application ID.

<div style="text-align: center; padding: 10pt;"><img src="./Media/EOS_Config/EOSPlugins.png" align="middle" width="450"></div>

Launch the Sharedspaces project.
Generate Visual Studio Project files and open the solution.
Build the project and run it.
Once you are in the editor go to Edit -> Plugins. 
Search for EOS and make sure everything with EOS is checked and restart the editor.

<div style="text-align: center; padding: 10pt;"><img src="./Media/EOS_Config/UnrealProjectSettings.png" align="middle" width="650"></div>

Go to Edit -> Project Settings -> Plugins -> OnlineSubsystem EOS.
Enter a Default Artifact Name.
Click the + on Artifacts and copy the Default Artifact name into Artifact Name.
Go to product settings on the EOS developer website and copy and paste the respective ids in the Artifacts fields.
For the Encryption Key, you need to enter 64 hex numbers.
