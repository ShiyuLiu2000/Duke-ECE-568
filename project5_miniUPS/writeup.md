### Project Differentiation Report

#### 1. Introduction

This report details the unique features implemented in our version of the "mini-Amazon" and "mini-UPS" systems as part of the ERSS project requirements. Our aim was to enhance user interaction, system efficiency, and overall reliability of the package delivery process.

#### 2. Differentiation Features

##### a. Enhanced User Communication

**Email Notification on Address Change:**
Whenever a user updates the delivery address for a package, our system automatically sends an email notification confirming the change. This ensures that the user is fully informed of the status and any modifications related to their shipment, thereby increasing transparency and user trust.

##### b. Role-Based Access Control (RBAC)

**Unauthenticated Access to Basic Package Information:**
Users without the need to log in can still access basic information about packages, such as tracking status and estimated delivery time. This feature allows for quick checks without compromising security, catering to users who need quick updates on the go.

**Authenticated User Benefits:**
Upon logging in, users gain access to more detailed functionalities. They can filter and view packages based on their current status—packed, loading, delivering, etc. This personalized dashboard enhances user experience by focusing on relevant information tailored to individual needs.

##### c. Intelligent Truck Dispatch System

**Optimized Truck Scheduling:**
Our system employs an algorithm that schedules trucks based on proximity to the warehouse and their current status (idle, delivering, etc.). This optimization reduces delivery times and increases the efficiency of our logistics network.

##### d. User Profile Management

**Editable User Email in Profile:**
Users can update their email addresses in their profile settings. This ensures that all notifications are sent to the correct email, keeping the user updated about their package status or any system announcements.


##### e. Synchronized Address Update Between UPS and Amazon

**Real-Time Address Synchronization:**
When a user changes their delivery address, our UPS system not only updates its own records but also communicates this change immediately to Amazon. This synchronization ensures that the modified address is updated across both platforms in real time. This feature enhances the coordination between the shipping and retail sides of our services, ensuring that both systems always reflect the most current and accurate information, thereby preventing any delivery errors due to address discrepancies.


#### 3. Functionalities

##### a. Tracking and Changing Package Destination

**Package Tracking:**
Logged-in users can track their packages using a unique tracking number assigned during the purchase. This feature provides detailed information about the package's journey and current status.

**Modifying Destination Before Loading:**
Users have the flexibility to change the delivery address of their package before it is loaded onto the truck. If a change request is made after the package is loaded, the system will promptly notify the user that the change is not possible, thereby avoiding confusion and potential errors.

##### b. Backend Integration

**Real-time Updates:**
Changes made by users on the frontend, such as address updates or email changes, are immediately processed by the backend. This synchronization ensures that the delivery process is updated in real time and that the package is delivered to the correct updated destination.

#### 4. Conclusion

Our differentiated features not only enhance the user experience but also improve the operational efficiency of the delivery system. By implementing these features, we aim to set a new standard in online retail and package delivery systems, ensuring that our project stands out in terms of functionality, user engagement, and service reliability.

