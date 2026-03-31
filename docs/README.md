# Secure IoT Sensor Node Documentation

## Overview

This documentation provides comprehensive technical documentation for the Secure IoT Sensor Node system, including interactive Mermaid diagrams, detailed architecture specifications, and deployment procedures.

## Quick Start

### Requirements
- **Browser**: Modern browser with JavaScript support
- **Network**: Internet connection for loading Mermaid.js library
- **Display**: Recommended resolution 1920x1080 or higher

### Accessing Documentation
1. Open `index.html` in web browser
2. Navigate using section tabs
3. Use diagram scaling controls for detailed viewing

## Documentation Structure

### Sections
- **Overview** - System architecture and capabilities
- **Architecture** - Detailed system design with sequence diagrams
- **Firmware** - Production-quality code and FreeRTOS task implementation
- **Web Dashboard** - Frontend components and API integration
- **Security** - Security measures and implementation practices
- **Deployment** - Build procedures and system startup

### Features
- **Interactive Diagrams** - Mermaid.js with dynamic scaling
- **Responsive Design** - Compatible with all device types
- **Smooth Transitions** - Tab-based navigation
- **Modern Interface** - Professional UI with gradient styling
- **Diagram Scaling** - 50%-200% zoom range

## Technology Stack

### Diagram Components
- **Mermaid.js 10.6.1** - Interactive diagram generation
- **SVG Rendering** - Vector-based graphics
- **Adaptive Sizing** - Automatic diagram scaling

### Frontend Technologies
- **HTML5/CSS3** - Modern web standards compliance
- **Vanilla JavaScript** - Lightweight client-side scripting
- **CSS Grid/Flexbox** - Adaptive layout systems
- **CSS Variables** - Dynamic theming support

### Design Implementation
- **Gradient Backgrounds** - Professional visual styling
- **Card Layouts** - Structured information presentation
- **Tab Navigation** - Intuitive content organization
- **Responsive Design** - Multi-device compatibility

---

## 📖 Using Documentation

### 🔄 Navigation
1. **Tabs** - Use top tabs for section switching
2. **Scaling** - Use +/-/⟲ buttons for diagram scaling
3. **Scrolling** - Long diagrams scroll within containers

### 🎯 Interactive Elements
- **Hover Effects** - Visual feedback on all interactive elements
- **Smooth Animations** - Fade-in transitions between tabs
- **Adaptive Scaling** - Diagram scaling from 50% to 200%
- **Mobile Support** - Compact view with vertical navigation

### 📱 Responsiveness
- **Desktop** - Full functionality with large diagrams
- **Tablet** - Optimized control placement
- **Mobile** - Compact view with vertical navigation

---

## 🔧 Customization

### 🎨 Changing Color Scheme
```css
:root {
    --primary-color: #667eea;
    --secondary-color: #764ba2;
    --background-gradient: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
}
```

### 📊 Configuring Mermaid
```javascript
await mermaid.initialize({
    theme: 'default',
    themeVariables: {
        primaryColor: '#667eea',
        primaryTextColor: '#333',
        lineColor: '#667eea'
    }
});
```

### 🎛️ Changing Scaling
```javascript
// In mermaid-height-util.js
constructor() {
    this.minScale = 0.3;     // Minimum scale
    this.maxScale = 3.0;     // Maximum scale
    this.currentScale = 1.0;  // Initial scale
}
```

---

## 📁 File Structure

```
docs/
├── index.html                 # Main documentation file
├── mermaid-height-util.js     # Scaling utility
├── README.md                 # This file
└── assets/                   # Images and resources (optional)
```

---

## 🌐 Deployment

### 📋 GitHub Pages
1. Upload files to repository
2. Enable GitHub Pages in settings
3. Select branch `main` and folder `/docs`
4. Documentation available at `https://username.github.io/repo/`

### 🐧 Local Server
```bash
# Python 3
python -m http.server 8000

# Node.js
npx http-server

# PHP
php -S localhost:8000
```

### 🐳 Docker
```dockerfile
FROM nginx:alpine
COPY docs/ /usr/share/nginx/html/
EXPOSE 80
CMD ["nginx", "-g", "daemon off;"]
```

---

## 🔍 Troubleshooting

### 📊 Diagrams Not Displaying
1. Check browser console for JavaScript errors
2. Ensure Mermaid.js loads from CDN
3. Verify Mermaid diagram syntax
4. Check connection to mermaid-height-util.js

### 🔧 Scaling Not Working
1. Ensure mermaid-height-util.js is loaded
2. Check for `.scale-controls` elements
3. Verify utility initialization in console

### 📱 Layout Issues
1. Check CSS styles for conflicts
2. Verify HTML structure is correct
3. Test responsiveness on different devices

### 🌐 Slow Loading
1. Check internet connection speed
2. Consider local Mermaid.js hosting
3. Optimize diagram sizes

---

## 📚 Additional Resources

### 📊 Mermaid.js
- [Official Documentation](https://mermaid-js.github.io/)
- [Syntax Guide](https://mermaid-js.github.io/mermaid/#/)
- [Theme Configuration](https://mermaid-js.github.io/mermaid/#/theming)

### 🎨 Web Design
- [CSS Grid Guide](https://css-tricks.com/snippets/css/complete-guide-grid/)
- [Flexbox Guide](https://css-tricks.com/snippets/css/a-guide-to-flexbox/)
- [Responsive Design](https://web.dev/responsive-web-design-basics/)

### 📱 Accessibility
- [ARIA Guidelines](https://www.w3.org/WAI/WCAG21/quickref/)
- [Semantic HTML](https://html5doctor.com/semantic-html5/)
- [Keyboard Navigation](https://web.dev/keyboard-accessibility/)

---

## 🔄 Updates and Maintenance

### 📅 Regular Updates
- Check for Mermaid.js updates
- Update dependencies when needed
- Test on new browsers
- Monitor performance

### 🐛 Monitoring
- Use analytics for usage tracking
- Collect user feedback
- Monitor performance
- Track errors

### 📝 Making Changes
1. Update content in `index.html`
2. Modify Mermaid diagrams when needed
3. Adapt styles in `<style>` block
4. Test on different devices

---

## 🎉 Conclusion

This documentation provides:
- **📊 Interactive Diagrams** for visual understanding of architecture
- **🎨 Modern Design** for professional presentation
- **📱 Responsiveness** for use on any device
- **🔧 Extensibility** for future updates
- **📚 Comprehensive Coverage** of all system aspects

Use this documentation as:
- **📖 Reference** for understanding system architecture
- **🎯 Guide** for developers and engineers
- **📊 Visualization** for presentations and demos
- **🔧 Instructions** for deployment and setup

---

**🚀 Enjoy exploring the Secure IoT Sensor Node!**
